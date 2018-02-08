#ifndef SRC_CLIENT_RANKCHECK_RANKCHECKWIDGET_HPP_
#define SRC_CLIENT_RANKCHECK_RANKCHECKWIDGET_HPP_

#include <Client/GUI3/Widget.hpp>
#include <Client/RankCheck/CountryLookup.hpp>
#include <Client/RankCheck/GameLogReader.hpp>
#include <Client/RankCheck/NetworkLogReader.hpp>
#include <Client/RankCheck/PersistentLogReader.hpp>
#include <Client/RankCheck/PlayerData.hpp>
#include <Client/RankCheck/PlayerDB.hpp>
#include <Client/RankCheck/RankChecker.hpp>
#include <Client/RankCheck/RatingHistoryEntry.hpp>
#include <Client/RankCheck/ReplayWatcher.hpp>
#include <Client/RankCheck/UsernameLookup.hpp>
#include <SFML/Config.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Clock.hpp>
#include <Shared/External/MinGWThreads/MinGWThreadsAuto.hpp>
#include <Shared/Utils/FileChooser.hpp>
#include <Shared/Utils/Timer.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sf
{
class RenderTarget;
}

namespace sf
{
class Font;
}

namespace gui3
{
class Panel;
}

class PlayerCard;

/**
 * "God class" managing all backend and frontend aspects of the RankCheck client.
 */
// TODO: split up this class into smaller components with individual responsibilities.
class RankCheckWidget : public gui3::Widget
{
public:

	RankCheckWidget();
	virtual ~RankCheckWidget();

	void requestPlayerDBRebuild();
	float getPlayerDBBuildProgressVisibility() const;
	std::string getPlayerDBBuildProgress() const;
	void reshowPlayers();
	void showGameDirChooser();
	bool isInitialInfoVisible() const;
	bool isPlayerDBBuildRunning() const;
	bool isAnimating() const;
	float getRatingBarHeight() const;

	void messageBox(std::string title, std::string text);
	void ask(std::string title, std::string question, std::function<void()> yes);
	void ask(std::string title, std::string question, std::function<void()> yes, std::function<void()> no);

private:

	virtual void onRepaint(gui3::Canvas & canvas) override;
	virtual void onRender(sf::RenderTarget & target, sf::RenderStates states) const override;

	void initGuiCallbacks();
	void initGameCallbacks();

	void handleConfigChange();
	void handleTick();

	gui3::Panel * getParentPanel() const;
	void dumpSharedAccounts();
	void dumpSponsoredAccounts();
	std::string userToString(sf::Uint64 steamID) const;

	void rebuildPlayerDB();
	bool rebuildPlayerDBFromDirectory(std::string directory);

	void readStartupNetlog(PlayerDB & db);

	void flushCurrentReplayDataToStats();

	bool updatePlayerCard(PlayerData data);
	void queueOrUpdatePlayerCard(PlayerData data);
	void queuePlayerCard(PlayerData data);
	void queuePlayerCard(PlayerData main, PlayerData alt);
	void updateAllPlayerCards();

	void updatePlayerDataFromLeaderboards(PlayerData & playerData, const RankChecker::Result & lbData);

	std::shared_ptr<PlayerCard> createPlayerCard(PlayerData data, bool subCard);
	int findFreeSlot(PlayerData::Team team) const;
	bool showPlayerCard(PlayerData data);
	bool showPlayerCard(PlayerData main, PlayerData alt);
	bool isSlotFree(int slot) const;

	void lookupCountryForCard(std::shared_ptr<PlayerCard> card);

	void queueRatingDiff(int diff);
	bool isRatingHistoryVisible() const;
	float getRatingHistoryFadeout() const;

	void loadFont();
	void updateScreenSize();

	int messageBoxCount = 0;
	RankChecker checker;
	sf::Clock timeSinceLastRequest;
	sf::Clock timeSinceLastScreenResize;
	ReplayWatcher replayWatcher;
	NetworkLogReader netlogReader;
	PersistentLogReader persistentReader;
	GameLogReader gamelogReader;
	UsernameLookup usernameLookup;
	CountryLookup countryLookup;
	std::unique_ptr<sf::Font> cardFont;
	std::vector<char> testFontData;
	std::string currentReplayPath;
	bool needToLoadDB = true;
	bool needRankRequest = false;
	bool needStartupNetlog = false;
	bool firstConfigPass = true;
	bool noMatchStartedYet = true;
	bool gameDirValid = false;
	bool exitOnGameDirChooserCancel = true;
	bool forceInstantCardDisplay = false;

	PlayerDB playerDBAsync;
	std::atomic_bool playerDBBuildRunning;
	std::atomic_bool playerDBBuildDone;
	std::atomic_int playerDBBuildDirCount;
	std::atomic_int playerDBBuildProgress;
	std::thread playerDBBuildThread;
	std::vector<std::string> playerDBReplayFolders;
	Countdown playerDBBuildSuccessTimer;

	FileChooser gameDirChooser;

	struct PendingCard
	{
		bool hasAlt;
		PlayerData main;
		PlayerData alt;
	};

	PlayerDB playerDB;
	std::vector<std::shared_ptr<PlayerCard> > playerCards;
	std::vector<PendingCard> pendingCards;

	RatingHistoryEntry currentScore;
	std::vector<RatingHistoryEntry> historyEntries;
	sf::Clock ratingHistoryTimer;

	PlayerData::Team localTeam = PlayerData::UnknownTeam;
};


#endif
