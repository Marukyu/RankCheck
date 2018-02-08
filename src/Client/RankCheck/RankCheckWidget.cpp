#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Widgets/Controls/Button.hpp>
#include <Client/GUI3/Widgets/Controls/Text.hpp>
#include <Client/GUI3/Widgets/Misc/Window.hpp>
#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <Client/RankCheck/GameFolder.hpp>
#include <Client/RankCheck/LeagueReader.hpp>
#include <Client/RankCheck/NautsNames.hpp>
#include <Client/RankCheck/NetworkLogStartupReader.hpp>
#include <Client/RankCheck/PlayerCard.hpp>
#include <Client/RankCheck/RankCheckWidget.hpp>
#include <Client/RankCheck/ReplayParser.hpp>
#include <Client/System/WOSApplication.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <utility>

static cfg::Bool disappearSimultaneously("rankcheck.playerPopups.disappearSimultaneously");
static cfg::Bool loadLeaderboards("rankcheck.loadLeaderboards");
static cfg::Bool loadCountries("rankcheck.loadCountries");
static cfg::Vector2f cardSize("rankcheck.playerPopups.cardSize");

static cfg::Bool ratingHistoryVisible("rankcheck.ratingHistory.visible");
static cfg::Bool ratingHistoryOnTop("rankcheck.ratingHistory.onTop");
static cfg::Float ratingHistoryTimeout("rankcheck.ratingHistory.timeout");
static cfg::Float ratingHistoryAnimTime("rankcheck.ratingHistory.animationTime");
static cfg::Int maxRatingCount("rankcheck.ratingHistory.bufferSize");
static cfg::Float ratingHistorySpace("rankcheck.ratingHistory.verticalSpace");

RankCheckWidget::RankCheckWidget()
{
	playerDBBuildRunning = false;
	playerDBBuildDone = false;
	playerDBBuildDirCount = 0;
	playerDBBuildProgress = 0;

	currentScore.setTotal(true);

	// Try to initially load non-empty player DB.
	try
	{
		playerDB.load();
		needToLoadDB = playerDB.empty();

		// Read startup netlogs only if player DB exists.
		// If player DB is empty, it will be read as part of the rebuilding process.
		needStartupNetlog = !playerDB.empty();
	}
	catch (std::exception & ex)
	{
	}

	gameDirChooser.setTitle("Choose Awesomenauts directory");

	setComplexOverride(true);

	initGameCallbacks();
	initGuiCallbacks();
}

RankCheckWidget::~RankCheckWidget()
{
	if (playerDBBuildRunning)
	{
		playerDBBuildThread.join();
		playerDB = playerDBAsync;
	}

	try
	{
		if (!playerDB.empty())
		{
			playerDB.save();
		}
	}
	catch (std::exception & ex)
	{
		debug() << ex.what();
	}
}

void RankCheckWidget::onRepaint(gui3::Canvas & canvas)
{

}

void RankCheckWidget::onRender(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto & card : playerCards)
	{
		target.draw(*card, states);
	}

	if (isRatingHistoryVisible())
	{
		float offset = getRatingBarHeight();
		float fadeout = config().get(ratingHistoryOnTop) ? 1 - getRatingHistoryFadeout() : getRatingHistoryFadeout();
		states.transform.translate(0, interpolateCosine<float>(0, offset, fadeout));
		for (const auto & entry : historyEntries)
		{
			target.draw(entry, states);
		}
		target.draw(currentScore, states);
	}
}

void RankCheckWidget::initGuiCallbacks()
{
	addStateCallback([=](gui3::StateEvent event)
	{
		handleConfigChange();
	}, gui3::StateEvent::ConfigChanged);

	addStateCallback([=](gui3::StateEvent event)
	{
		loadFont();
		for (auto & card : playerCards)
		{
			card->setApplication(getParentApplication());
		}
	}, gui3::StateEvent::ParentApplicationChanged);

	addStateCallback([=](gui3::StateEvent event)
	{
		timeSinceLastScreenResize.restart();
		updateScreenSize();
	}, gui3::StateEvent::Resized);

	addTickCallback([=]()
	{
		handleTick();
	});
}

void RankCheckWidget::initGameCallbacks()
{
	replayWatcher.setReplayStartCallback([=](std::string replayPath)
	{
		currentReplayPath = replayPath;
	});

	netlogReader.setCallbackStart([=]()
	{
		persistentReader.resetPlayers();
		persistentReader.markMatchStart();
		persistentReader.saveState(config());

		localTeam = PlayerData::UnknownTeam;
		pendingCards.clear();
		for (auto & card : playerCards)
		{
			card->kill();
		}
		loadFont();
	});

	netlogReader.setCallbackEnd([=]()
	{
		pendingCards.clear();
		flushCurrentReplayDataToStats();
	});

	persistentReader.setPlayerCallback([=](sf::Uint64 steamID)
	{
		if (steamID == 0)
		{
			steamID = netlogReader.getLocalSteamID();
		}

		PlayerData player;
		player.steamID = steamID;
		player.ip = sf::IpAddress::None;
		player.team = PlayerData::Blue;
		player.isLocal = (netlogReader.getLocalSteamID() == player.steamID);
		playerDB.fillPlayerData(player);
		queuePlayerCard(player);
	});

	persistentReader.setRatingCallback([=](sf::Int64 diff)
	{
		queueRatingDiff(diff);
		currentScore.setValue(persistentReader.getCurrentRating());
	});
}

void RankCheckWidget::handleConfigChange()
{
	static cfg::String lbHost("rankcheck.servers.leaderboard.host");
	static cfg::Int lbPort("rankcheck.servers.leaderboard.port");
	static cfg::String lbUriPrefix("rankcheck.servers.leaderboard.uriPrefix");
	static cfg::String lbUriSeparator("rankcheck.servers.leaderboard.separator");
	static cfg::String lbUriSuffix("rankcheck.servers.leaderboard.uriSuffix");

	static cfg::String ctryHost("rankcheck.servers.country.host");
	static cfg::Int ctryPort("rankcheck.servers.country.port");
	static cfg::String ctryUriPrefix("rankcheck.servers.country.uriPrefix");
	static cfg::String ctryUriSuffix("rankcheck.servers.country.uriSuffix");

	NautsNames::getInstance().initWithConfig(config());
	try
	{
		GameFolder::getInstance().initWithConfig(config());
		gameDirValid = true;
		exitOnGameDirChooserCancel = false;
	}
	catch (std::exception & ex)
	{
		gameDirValid = false;
		showGameDirChooser();
	}

	if (needStartupNetlog)
	{
		needStartupNetlog = false;
		readStartupNetlog(playerDB);
	}

	LeagueReader::getInstance().initWithConfig(config());
	checker.setHost(config().get(lbHost), config().get(lbPort));
	checker.setUriParameters(config().get(lbUriPrefix), config().get(lbUriSeparator), config().get(lbUriSuffix));
	countryLookup.setHost(config().get(ctryHost), config().get(ctryPort));
	countryLookup.setUriParameters(config().get(ctryUriPrefix), config().get(ctryUriSuffix));
	replayWatcher.initWithConfig(config());
	netlogReader.initWithConfig(config());
	gamelogReader.initWithConfig(config());
	persistentReader.initWithConfig(config());

	for (auto & card : playerCards)
	{
		card->initWithConfig(config());
	}

	for (auto & histEntry : historyEntries)
	{
		histEntry.initWithConfig(config());
	}
	currentScore.initWithConfig(config());

	updateScreenSize();

	firstConfigPass = false;
}

void RankCheckWidget::handleTick()
{
	if (playerDBBuildDone)
	{
		playerDBBuildDone = false;
		playerDBBuildRunning = false;
		playerDBBuildThread.detach();
		playerDBBuildSuccessTimer.restart(sf::seconds(10));
		playerDB = playerDBAsync;
		playerDB.save();
		updateAllPlayerCards();
	}

	if (gameDirChooser.isDone())
	{
		static cfg::String gameDirKey("rankcheck.awesomenauts.gameFolder");
		config().set(gameDirKey, gameDirChooser.getSelectedFile());
		fireStateEvent(gui3::StateEvent::ConfigChanged);
		if (getParentApplication())
		{
			WOSApplication * wosapp = dynamic_cast<WOSApplication *>(getParentApplication());
			if (wosapp)
			{
				wosapp->saveConfig();
			}
		}
		gameDirChooser.clear();
	}

	if (gameDirChooser.isCancelled() && exitOnGameDirChooserCancel)
	{
		getParentApplication()->close(getParentInterface());
		return;
	}

	if (!gameDirChooser.isOpen() && needToLoadDB && getParentPanel() != nullptr)
	{
		needToLoadDB = false;

		try
		{
			playerDB.load();
		}
		catch (std::exception & ex)
		{
			debug() << "Player DB corrupted or missing: " << ex.what();
		}

		if (playerDB.empty())
		{
			rebuildPlayerDB();
		}
	}

	for (auto & card : playerCards)
	{
		card->autoShow();
	}

	netlogReader.process();
	gamelogReader.process();
	persistentReader.setLocalSteamID(netlogReader.getLocalSteamID());
	persistentReader.process();
	countryLookup.process();
	usernameLookup.process();
	replayWatcher.process();
	checker.setLocalSteamID(netlogReader.getLocalSteamID());
	checker.sendRequestIfNeeded();
	checker.process();

	if (persistentReader.checkStateSaveRequest())
	{
		persistentReader.saveState(config());
	}

	playerCards.erase(std::remove_if(playerCards.begin(), playerCards.end(),
			[](const std::shared_ptr<PlayerCard> & card)
			{
				return card->done();
			}), playerCards.end());

	if (forceInstantCardDisplay || (gamelogReader.isInGame() && netlogReader.isSessionActive()))
	{
		for (auto it = pendingCards.begin(); it != pendingCards.end(); )
		{
			const auto & pc = *it;

			bool retVal = false;
			if (pc.hasAlt)
			{
				retVal = showPlayerCard(pc.main, pc.alt);
			}
			else
			{
				retVal = showPlayerCard(pc.main);
			}

			if (retVal)
			{
				it = pendingCards.erase(it);
			}
			else
			{
				++it;
			}
		}
		forceInstantCardDisplay = false;
	}
}

gui3::Panel* RankCheckWidget::getParentPanel() const
{
	if (getParent() == nullptr || getParent()->getParent() == nullptr)
	{
		return nullptr;
	}

	return dynamic_cast<gui3::Panel *>(getParent()->getParent());
}

static const float messageBoxWidth = 400.f;

void RankCheckWidget::messageBox(std::string title, std::string text)
{
	if (!getParentPanel())
	{
		return;
	}

	messageBoxCount++;

	float messageBoxHeight = 17 * (std::count(text.begin(), text.end(), '\n') + 1) + 60;

	auto tex = gui3::make<gui3::Text>();
	tex->setText(text);
	tex->setRect(10, 10, messageBoxWidth - 20, messageBoxHeight - 25 - 10 - 10);
	tex->setTextAlignment(0.5, 0);

	auto button = gui3::make<gui3::Button>();
	button->setText("OK");
	button->setSize(100, 25);
	button->setPosition((messageBoxWidth - 100) / 2.f, messageBoxHeight - 25 - 10);

	auto win = gui3::make<gui3::Window>();
	gui3::Window * winP = win.get();
	getParentPanel()->add(win);
	win->setTitle(title);
	win->setResizable(false);
	win->setClosable(true);
	win->setMaximizable(false);
	win->setContentSize(messageBoxWidth, messageBoxHeight);
	win->setPosition(sf::Vector2f(sf::Vector2i((getParentPanel()->getSize() - win->getSize()) / 2.f)));
	win->setZPosition(10);
	win->addStateCallback([=](gui3::StateEvent event)
	{
		if (!winP->isVisible() && getParentPanel() != nullptr)
		{
			messageBoxCount--;
			getParentPanel()->remove(*winP);
		}
	}, gui3::StateEvent::VisibilityChanged);
	button->addActionCallback([=]()
	{
		winP->setVisible(false);
	});

	win->add(tex);
	win->add(button);
}

void RankCheckWidget::ask(std::string title, std::string question, std::function<void()> yes)
{
	ask(title, question, yes, [](){});
}

void RankCheckWidget::ask(std::string title, std::string question, std::function<void()> yes, std::function<void()> no)
{
	if (!getParentPanel())
	{
		return;
	}

	messageBoxCount++;

	float messageBoxHeight = 17 * (std::count(question.begin(), question.end(), '\n') + 1) + 60;

	auto tex = gui3::make<gui3::Text>();
	tex->setText(question);
	tex->setRect(10, 10, messageBoxWidth - 20, messageBoxHeight - 25 - 10 - 10);
	tex->setTextAlignment(0.5, 0);

	auto buttonY = gui3::make<gui3::Button>();
	buttonY->setText("Yes");
	buttonY->setSize(100, 25);
	buttonY->setPosition((messageBoxWidth - 100) / 2.f - 55, messageBoxHeight - 25 - 10);

	auto buttonN = gui3::make<gui3::Button>();
	buttonN->setText("No");
	buttonN->setSize(100, 25);
	buttonN->setPosition((messageBoxWidth - 100) / 2.f + 55, messageBoxHeight - 25 - 10);

	auto win = gui3::make<gui3::Window>();
	gui3::Window * winP = win.get();
	getParentPanel()->add(win);
	win->setTitle(title);
	win->setResizable(false);
	win->setClosable(true);
	win->setMaximizable(false);
	win->setContentSize(messageBoxWidth, messageBoxHeight);
	win->setPosition(sf::Vector2f(sf::Vector2i((getParentPanel()->getSize() - win->getSize()) / 2.f)));
	win->setZPosition(10);
	win->addStateCallback([=](gui3::StateEvent event)
	{
		if (!winP->isVisible() && getParentPanel() != nullptr)
		{
			messageBoxCount--;
			getParentPanel()->remove(*winP);
		}
	}, gui3::StateEvent::VisibilityChanged);
	buttonY->addActionCallback([=]()
	{
		yes();
		winP->setVisible(false);
	});
	buttonN->addActionCallback([=]()
	{
		no();
		winP->setVisible(false);
	});

	win->add(tex);
	win->add(buttonY);
	win->add(buttonN);
}

void RankCheckWidget::dumpSharedAccounts()
{
	debug() << "Shared accounts:";
	std::map<sf::IpAddress, std::vector<sf::Uint64> > ips;
	std::set<sf::IpAddress> multiIPs;
	for (auto it = playerDB.getEntries().begin(); it != playerDB.getEntries().end(); ++it)
	{
		for (auto ip : it->second.ipAddresses)
		{
			ips[ip].push_back(it->first);
			if (ips[ip].size() > 1)
			{
				multiIPs.insert(ip);
			}
		}
	}
	for (auto ip : multiIPs)
	{
		auto d = debug();
		for (auto steamID : ips[ip])
		{
			d << userToString(steamID) << " ";
		}
	}
	debug();
}

void RankCheckWidget::dumpSponsoredAccounts()
{
	debug() << "Sponsored accounts:";
	for (auto it = playerDB.getEntries().begin(); it != playerDB.getEntries().end(); ++it)
	{
		if (it->second.sponsorSteamID != 0)
		{
			debug() << userToString(it->first) << " " << userToString(it->second.sponsorSteamID);
		}
	}
	debug();
}

std::string RankCheckWidget::userToString(sf::Uint64 steamID) const
{
	if (playerDB.getEntry(steamID).getCommonName().empty())
	{
		return "[unknown]#" + cNtoS(steamID);
	}
	else
	{
		return playerDB.getEntry(steamID).getCommonName() + "#" + cNtoS(steamID);
	}
}

void RankCheckWidget::rebuildPlayerDB()
{
	playerDB.clear();

	if (!playerDBBuildRunning)
	{
		static cfg::Int replayFoldersLength("rankcheck.awesomenauts.replayFolders.length");

		playerDBReplayFolders.clear();

		for (unsigned int i = 0; i < config().get(replayFoldersLength); ++i)
		{
			cfg::String replayFolderEntry("rankcheck.awesomenauts.replayFolders[" + cNtoS(i) + "]");
			std::string replayFolder = GameFolder::getInstance().getFolder() + "/Data/"
				+ config().get(replayFolderEntry);

			if (isDirectory(replayFolder))
			{
				playerDBReplayFolders.push_back(replayFolder);
			}
		}

		if (playerDBReplayFolders.empty())
		{
			messageBox("Error", std::string("Failed to build player database: no valid directories specified.\n"
				"Please adjust or redownload rankcheck.json."));
		}
		else
		{
			playerDBAsync = playerDB;

			playerDBBuildProgress = 0;
			playerDBBuildDirCount = 0;
			playerDBBuildDone = false;
			playerDBBuildRunning = true;
			playerDBBuildThread = std::thread([this]()
			{
				for (const auto & folder : playerDBReplayFolders)
				{
					rebuildPlayerDBFromDirectory(folder);
				}
				readStartupNetlog(playerDBAsync);
				playerDBBuildDone = true;
			});
		}
	}
}

bool RankCheckWidget::rebuildPlayerDBFromDirectory(std::string directory)
{
	if (!isDirectory(directory))
	{
		return false;
	}

	std::vector<std::string> dirs;
	listDirectories(directory, dirs, false, true);

	playerDBBuildProgress = 0;
	playerDBBuildDirCount = dirs.size();

	for (std::string dir : dirs)
	{
		ReplayParser parser(directory + "/" + dir + "/Replays.info");
		ReplayParser::ReplayInfo info = parser.parse();
		if (info.countStats && !playerDBAsync.hasReplayHash(info.hash))
		{
			playerDBAsync.addReplayHash(info.hash);
			for (const auto & player : info.players)
			{
				playerDBAsync.addPlayerToStats(player.player, info.localTeam, player.sponsor.steamID);
			}
		}
		playerDBBuildProgress++;
	}

	return true;
}

void RankCheckWidget::flushCurrentReplayDataToStats()
{
	if (!currentReplayPath.empty())
	{
		ReplayParser parser(currentReplayPath + "/Replays.info");
		ReplayParser::ReplayInfo info = parser.parse();
		if (info.countStats && !playerDB.hasReplayHash(info.hash))
		{
			playerDB.addReplayHash(info.hash);
			for (const auto & player : info.players)
			{
				debug() << "Adding player to stats: " << player.player.currentName;
				playerDB.addPlayerToStats(player.player, info.localTeam, player.sponsor.steamID);
			}
		}

		updateAllPlayerCards();
	}
}

bool RankCheckWidget::updatePlayerCard(PlayerData data)
{
	noMatchStartedYet = false;
	bool found = false;
	for (auto card : playerCards)
	{
		if (card->getPlayerData().steamID == data.steamID)
		{
			if (card->getPlayerData().ip != data.ip)
			{
				lookupCountryForCard(card);
			}
			card->setPlayerData(data);
			found = true;
		}
	}
	return found;
}

void RankCheckWidget::queueOrUpdatePlayerCard(PlayerData data)
{
	if (!updatePlayerCard(data))
	{
		pendingCards.push_back( { false, data, {} });
	}
}

void RankCheckWidget::queuePlayerCard(PlayerData data)
{
	noMatchStartedYet = false;
	pendingCards.push_back( { false, data, {} });
}

void RankCheckWidget::queuePlayerCard(PlayerData main, PlayerData alt)
{
	noMatchStartedYet = false;
	alt.type = PlayerData::Player;
	pendingCards.push_back( { false, alt, {} });
	//pendingCards.push_back( { true, main, alt });
}

bool RankCheckWidget::showPlayerCard(PlayerData data)
{
	auto card = createPlayerCard(data, false);

	if (!card)
	{
		return false;
	}

	playerCards.push_back(card);

	updateScreenSize();

	if (config().get(disappearSimultaneously))
	{
		for (auto & card : playerCards)
		{
			card->resetTimer();
		}
	}

	return true;
}

bool RankCheckWidget::showPlayerCard(PlayerData main, PlayerData alt)
{
	auto altCard = createPlayerCard(alt, false);

	if (!altCard)
	{
		return false;
	}

	auto mainCard = createPlayerCard(main, true);

	altCard->setSubCard(mainCard);
	altCard->setApplication(getParentApplication());
	altCard->initWithConfig(config());

	playerCards.push_back(altCard);

	updateScreenSize();

	if (config().get(disappearSimultaneously))
	{
		for (auto & card : playerCards)
		{
			card->resetTimer();
		}
	}

	return true;
}

std::shared_ptr<PlayerCard> RankCheckWidget::createPlayerCard(PlayerData data, bool subCard)
{
	int freeSlot = findFreeSlot(data.team);

	if (!isSlotFree(freeSlot) && !subCard)
	{
		return nullptr;
	}

	std::shared_ptr<PlayerCard> card = std::make_shared<PlayerCard>();
	card->setApplication(getParentApplication());
	card->setSlot(freeSlot);
	card->setScreenSize(getSize());
	card->setFont(*cardFont);
	card->setPlayerData(data);
	card->initWithConfig(config());

	lookupCountryForCard(card);

	if (data.steamID != 0)
	{
		usernameLookup.lookup(data.steamID, [=](std::string username)
		{
			PlayerData pd = card->getPlayerData();
			pd.currentName = username;
			card->setPlayerData(pd);
		});

		if (config().get(loadLeaderboards))
		{
			checker.addSteamIDRequest(data.steamID, [=](RankChecker::Result result)
			{
				PlayerData pd = card->getPlayerData();
				updatePlayerDataFromLeaderboards(pd, result);
				card->setPlayerData(pd);
				card->show();
			});
			needRankRequest = true;
		}

		timeSinceLastRequest.restart();
	}

	return card;
}

int RankCheckWidget::findFreeSlot(PlayerData::Team team) const
{
	static cfg::Int maxCardCountPerTeam("rankcheck.playerPopups.maxCardCountPerTeam");

	int maxCardCount = config().get(maxCardCountPerTeam);

	for (int slot = 1; slot <= maxCardCount; ++slot)
	{
		if (isSlotFree(-slot))
		{
			return -slot;
		}
		else if (isSlotFree(slot))
		{
			return slot;
		}
	}

	return 0;
}

bool RankCheckWidget::isSlotFree(int slot) const
{
	if (slot == 0)
	{
		return false;
	}
	for (const auto & card : playerCards)
	{
		if (card->getSlot() == slot)
		{
			return false;
		}
	}
	return true;
}

void RankCheckWidget::requestPlayerDBRebuild()
{
	ask("Rebuild player database",
		"Do you wish to rebuild the player database?\n\n"
		"This recalculates the number of ally/enemy\n"
		"encounters and previously used nicknames.",
		[=]()
		{
			rebuildPlayerDB();
		});
}

float RankCheckWidget::getPlayerDBBuildProgressVisibility() const
{
	return playerDBBuildRunning ? 1.f :
		std::min(playerDBBuildSuccessTimer.getRemainingTime().asSeconds(), 1.f);
}

std::string RankCheckWidget::getPlayerDBBuildProgress() const
{
	if (playerDBBuildRunning)
	{
		if (playerDBBuildDirCount == 0)
		{
			return "Scanning replay folder...";
		}
		else
		{
			return "Building player database... (" + cNtoS(playerDBBuildProgress * 100 / playerDBBuildDirCount) + "%)";
		}
	}
	else if (!playerDBBuildSuccessTimer.expired())
	{
		return "Player database was built successfully!\n"
			"Number of unique matches: " + cNtoS(playerDB.getReplayHashCount()) + "\n"
			"Number of unique players: " + cNtoS(playerDB.getPlayerCount());
	}
	else
	{
		return "";
	}
}

bool RankCheckWidget::isPlayerDBBuildRunning() const
{
	return playerDBBuildRunning;
}

bool RankCheckWidget::isAnimating() const
{
	if (timeSinceLastScreenResize.getElapsedTime() < sf::seconds(3))
	{
		return true;
	}

	for (auto & card : playerCards)
	{
		if (card->isAnimating())
		{
			return true;
		}
	}

	if (isRatingHistoryVisible())
	{
		for (auto & entry : historyEntries)
		{
			if (entry.isAnimating())
			{
				return true;
			}
		}

		float fade = getRatingHistoryFadeout();
		if (fade > 0.0001 && fade < 0.999)
		{
			return true;
		}
	}
	return false;
}

void RankCheckWidget::lookupCountryForCard(std::shared_ptr<PlayerCard> card)
{
	if (config().get(loadCountries))
	{
		sf::IpAddress ip = card->getPlayerData().isLocal ? sf::IpAddress::LocalHost : card->getPlayerData().ip;
		countryLookup.lookup(ip, [this,card](std::string countryCode)
		{
			PlayerData pd = card->getPlayerData();
			pd.countryCode = countryCode;
			pd.country = config().get(cfg::String("countries." + countryCode));
			if (pd.country.empty())
			{
				pd.country = config().get(cfg::String("countries.XX"));
			}
			card->setPlayerData(pd);
		});
	}
}

void RankCheckWidget::updateAllPlayerCards()
{
	for (auto card : playerCards)
	{
		PlayerData data = card->getPlayerData();
		playerDB.fillPlayerData(data);
		updatePlayerCard(data);
	}
}

void RankCheckWidget::updatePlayerDataFromLeaderboards(PlayerData& playerData, const RankChecker::Result& lb)
{
	if (lb.code != RankChecker::Result::Success)
	{
		playerData.downloadFailed = true;
		debug() << "Leaderboard request for " << userToString(playerData.steamID) << " timed out.";
		return;
	}

	if (lb.scoredata.size() < 13)
	{
		playerData.downloadFailed = true;
		debug() << "Unexpected scoredata size for " << userToString(playerData.steamID) << ": " << lb.scoredata.size();
		return;
	}

	playerData.hasLeaderboardData = true;
	playerData.rank = lb.scoredata[0];
	playerData.rating = lb.scoredata[1];
	// Skip version (scoredata[2])
	playerData.winCountTotal = lb.scoredata[3];
	playerData.lossCountTotal = lb.scoredata[4];
	// Skip killcount (scoredata[5])
	// Skip deathcount (scoredata[6])
	// Skip prestige (scoredata[7])
	playerData.mainNaut = lb.scoredata[8];
	playerData.winCount = lb.scoredata[9];
	playerData.lossCount = lb.scoredata[10];
	// Skip contrib1 (lb.scoredata[11])
	// Skip contrib2 (lb.scoredata[12])

	playerData.league = LeagueReader::getInstance().getLeagueForRank(playerData.rank);
}

void RankCheckWidget::readStartupNetlog(PlayerDB & db)
{
	NetworkLogStartupReader reader;
	reader.readNetworkLogs(config());
	for (const auto & mapping : reader.getIPMappings())
	{
		db.addIPMapping(mapping.steamID, mapping.ip);
	}
}

void RankCheckWidget::queueRatingDiff(int diff)
{
	ratingHistoryTimer.restart();

	for (auto & entry : historyEntries)
	{
		entry.setSlot(entry.getSlot() + 1);
	}

	RatingHistoryEntry entry;
	entry.setSlot(1);
	entry.setValue(diff);
	entry.setFont(*cardFont);
	entry.initWithConfig(config());
	historyEntries.push_back(entry);

	while ((sf::Int64)historyEntries.size() > config().get(maxRatingCount))
	{
		historyEntries.erase(historyEntries.begin());
	}

	updateScreenSize();
}

void RankCheckWidget::loadFont()
{
	if (getParentApplication() == nullptr)
	{
		return;
	}

	static const std::string fontResName = "rankcheck/NotoSans-Bold.ttf";
	auto res = resources().acquireData(fontResName);
	if (res == nullptr)
	{
		throw Error("Failed to find font resource " + fontResName);
	}
	auto font = makeUnique<sf::Font>();
	if (!font->loadFromMemory(res->getData(), res->getDataSize()))
	{
		throw Error("Failed to load font " + fontResName);
	}

	cardFont = std::move(font);
	for (auto card : playerCards)
	{
		card->setFont(*cardFont);
	}
	for (auto & entry : historyEntries)
	{
		entry.setFont(*cardFont);
	}
	currentScore.setFont(*cardFont);
}

void RankCheckWidget::reshowPlayers()
{
	loadFont();

	pendingCards.clear();

	const auto & playerList = persistentReader.getCurrentPlayers().empty() ?
		persistentReader.getPreviousPlayers() : persistentReader.getCurrentPlayers();

	for (auto steamID : playerList)
	{
		if (steamID == 0)
		{
			steamID = netlogReader.getLocalSteamID();
		}

		PlayerData player;
		player.steamID = steamID;
		for (auto card : playerCards)
		{
			if (card->getPlayerData().steamID == player.steamID)
			{
				player = card->getPlayerData();
				break;
			}
		}
		//player.ip = info.ip;
		player.team = PlayerData::Blue;
		player.isLocal = (netlogReader.getLocalSteamID() == player.steamID);
		playerDB.fillPlayerData(player);
		queueOrUpdatePlayerCard(player);
	}

	for (auto card : playerCards)
	{
		bool found = false;
		for (auto steamID : playerList)
		{
			if (steamID == 0)
			{
				steamID = netlogReader.getLocalSteamID();
			}

			if (steamID == card->getPlayerData().steamID)
			{
				debug() << "Keeping player card for " << userToString(steamID);
				found = true;
				break;
			}
		}

		if (!found)
		{
			debug() << "Removing player card for " << userToString(card->getPlayerData().steamID);
			card->kill();
		}
	}

	forceInstantCardDisplay = true;
}

void RankCheckWidget::showGameDirChooser()
{
	gameDirChooser.setDefaultPath(GameFolder::getInstance().getFolder());
	gameDirChooser.showDialog(FileChooser::OpenFolder);
}

bool RankCheckWidget::isInitialInfoVisible() const
{
	return noMatchStartedYet && gameDirValid;
}

bool RankCheckWidget::isRatingHistoryVisible() const
{
	float timeout = config().get(ratingHistoryTimeout);
	float animSpeed = config().get(ratingHistoryAnimTime);
	return config().get(ratingHistoryVisible)
		&& (timeout < 1.f || ratingHistoryTimer.getElapsedTime().asSeconds() < timeout + animSpeed);
}

float RankCheckWidget::getRatingHistoryFadeout() const
{
	float timeout = config().get(ratingHistoryTimeout);
	float animSpeed = config().get(ratingHistoryAnimTime);

	if (timeout < 1.f)
	{
		return 0;
	}
	else if (animSpeed == 0)
	{
		return isRatingHistoryVisible() ? 0 : 1;
	}
	else
	{
		return clamp(0.f, (ratingHistoryTimer.getElapsedTime().asSeconds() - timeout) / animSpeed, 1.f);
	}
}

float RankCheckWidget::getRatingBarHeight() const
{
	if (config().get(ratingHistoryVisible))
	{
		return config().get(ratingHistorySpace) * getSize().y;
	}
	else
	{
		return 0;
	}
}

void RankCheckWidget::updateScreenSize()
{
	float fac = config().get(ratingHistoryVisible) ? config().get(ratingHistorySpace) : 0;
	for (auto & card : playerCards)
	{
		card->setScreenSize({getSize().x, getSize().y * (1 - fac)});
		card->setPosition(0, config().get(ratingHistoryOnTop) ? getSize().y * fac : 0);
	}
	for (auto & histEntry : historyEntries)
	{
		histEntry.setScreenSize({getSize().x, getSize().y * fac});
		histEntry.setPosition(getSize().x, config().get(ratingHistoryOnTop) ? 0 : getSize().y);
	}
	currentScore.setScreenSize({getSize().x, getSize().y * fac});
	currentScore.setPosition(getSize().x, config().get(ratingHistoryOnTop) ? 0 : getSize().y);
}
