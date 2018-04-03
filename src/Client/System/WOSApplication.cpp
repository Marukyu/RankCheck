#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <Client/System/WOSApplication.hpp>
#include <Client/System/WOSInterface.hpp>
#include <Client/System/WOSResourceManager.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Config/ConfigAggregator.hpp>
#include <Shared/Config/JSONConfig.hpp>
#include <Shared/Content/Package.hpp>
#include <Shared/Utils/DataStream.hpp>
#include <Shared/Utils/Debug/CrashHandler.hpp>
#include <Shared/Utils/Debug/StackTrace.hpp>
#include <Shared/Utils/Debug/TypeInfo.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/StringStream.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/SystemMessage.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <algorithm>
#include <iostream>

static const std::string builtinConfigResname = "rankcheck/rankcheck-data.json";
static const std::string userConfigFilename = "rankcheck.json";
static const std::string applicationConfigFilename = "rankcheck.cfg";
static const std::string assetsFilename = "rankcheck.dat";
static const std::string assetsDirectory = "assets";

static cfg::Color backgroundColor("rankcheck.backgroundColor");
static cfg::Bool showInitialInfo("rankcheck.showInitialInfoMessage");
static cfg::Bool ratingHistoryVisible("rankcheck.ratingHistory.visible");
static cfg::Bool ratingHistoryOnTop("rankcheck.ratingHistory.onTop");
static cfg::Float playerPopupTimeout("rankcheck.playerPopups.timeout");
static cfg::Float ratingHistoryTimeout("rankcheck.ratingHistory.timeout");
static cfg::Float focusedFrameRate("rankcheck.framerate.focused");
static cfg::Float unfocusedFrameRate("rankcheck.framerate.unfocused");


WOSApplication::WOSApplication()
{
}

WOSApplication::~WOSApplication()
{
	saveConfig();
}

const sf::Texture* WOSApplication::getTexture(std::size_t pageIndex) const
{
	return myResourceManager->getTexture(pageIndex);
}

gui3::res::ResourceManager & WOSApplication::getResourceManager() const
{
	return *myResourceManager;
}

sf::Vector2f WOSApplication::getWhitePixel() const
{
	return myWhitePixelPosition;
}

cfg::Config& WOSApplication::getConfig() const
{
	return *myConfig;
}

void WOSApplication::saveConfig()
{
	if (myApplicationConfig == nullptr)
	{
		return;
	}

	DataStream stream;
	stream.openOutFile(applicationConfigFilename, false);
	std::string configContents = myApplicationConfig->saveToString(cfg::JSONConfig::Style::Pretty);
	stream.addData(configContents.data(), configContents.size());
}

int WOSApplication::init(const std::vector<std::string>& args)
{
	try
	{
		initCrashHandler();
		initAssets();
		initConfig();
		initWhitePixel();
		initWindow();
		return LoadSuccess;
	}
	catch (std::exception & ex)
	{
		handleError(ex);
		return LoadError;
	}
}

void WOSApplication::handleError(std::exception& exception)
{
	displayError(exception);

	for (gui3::Interface * interface : getOpenInterfaces())
	{
		close(interface);
	}
}

void WOSApplication::displayError(std::exception& exception, std::string errorDescription)
{
	std::string errorType;
	std::string shortError;
	std::string longError;

	Error * castError = dynamic_cast<Error *>(&exception);
	if (castError != nullptr)
	{
		errorType = "RankCheck Error";
		shortError = castError->getErrorText();
		longError = castError->getStackTrace().toString();
	}
	else
	{
		errorType = TypeInfo<std::exception>(exception).name();
		shortError = exception.what();
	}

	if (errorDescription.empty())
	{
		writeDebugOutput(errorType + ": " + shortError + "\n" + longError);
	}
	else
	{
		writeDebugOutput(errorType + " (" + errorDescription + "): " + shortError + "\n" + longError);
	}

	std::cerr << errorType << ": " << shortError << "\n" << longError << std::endl;
	SystemMessage::showErrorMessage(errorType, shortError + "\n\nSee error.log for more details");
}

void WOSApplication::initCrashHandler()
{
	CrashHandler::getInstance().registerSegfaultHandler([=]()
	{
		auto trace = StackTrace::generate();
		writeDebugOutput(trace.toString());

		Error error("RankCheck has crashed.");
		displayError(error, "Segmentation violation");
	});
}

void WOSApplication::initAssets()
{
	std::unique_ptr<Package> package = makeUnique<Package>();
	if (!package->openFile(assetsFilename))
	{
		throw Error("Failed to open file " + assetsFilename + ".\n"
			"Please ensure the file is present in the same folder as the RankCheck executable.");
	}

	if (myResourceManager == nullptr)
	{
		myResourceManager = makeUnique<WOSResourceManager>();
	}

	myResourceManager->setPackage(std::move(package));
}

void WOSApplication::initConfig()
{
	if (myConfig == nullptr)
	{
		myConfig = makeUnique<cfg::Config>();
	}

	if (myConfigAggregator == nullptr)
	{
		myConfigAggregator = std::make_shared<cfg::ConfigAggregator>();
	}

	if (myBuiltinConfig == nullptr)
	{
		myBuiltinConfig = std::make_shared<cfg::JSONConfig>();
		myConfigAggregator->addConfig(myBuiltinConfig, 0);
	}

	if (myUserConfig == nullptr)
	{
		myUserConfigReloader.setTargetFile(userConfigFilename);
		myUserConfig = std::make_shared<cfg::JSONConfig>();
		myConfigAggregator->addConfig(myUserConfig, -1);
	}

	if (myApplicationConfig == nullptr)
	{
		myApplicationConfig = std::make_shared<cfg::JSONConfig>();
		myConfigAggregator->addConfig(myApplicationConfig, -2);
		myConfigAggregator->setWritableConfig(myApplicationConfig.get());
	}

	if (myConfig->getConfigSource() == nullptr)
	{
		myConfig->setConfigSource(myConfigAggregator);
	}

	myConfig->clearCache();

	try
	{
		auto builtinCfgData = getResourceManager().acquireData(builtinConfigResname);
		if (builtinCfgData == nullptr)
		{
			throw Error("Failed to open " + builtinConfigResname);
		}
		myBuiltinConfig->loadFromMemory(builtinCfgData->getData(), builtinCfgData->getDataSize());
	}
	catch (std::exception & ex)
	{
		displayError(ex, "Error loading " + builtinConfigResname);
	}

	try
	{
		DataStream configStream;
		if (!configStream.openInFile(userConfigFilename))
		{
			throw Error("Failed to open " + userConfigFilename);
		}
		myUserConfig->loadFromMemory((const char *) configStream.getData(), configStream.getDataSize());
	}
	catch (std::exception & ex)
	{
		displayError(ex, "Error loading " + userConfigFilename);
	}

	try
	{
		DataStream configStream;
		if (configStream.openInFile(applicationConfigFilename))
		{
			myApplicationConfig->loadFromMemory((const char *) configStream.getData(), configStream.getDataSize());
		}
	}
	catch (std::exception & ex)
	{
		// Ignore error. Config doesn't have to exist.
	}
}

void WOSApplication::initWhitePixel()
{
	myWhitePixel = getResourceManager().acquireImage(WOSResourceManager::IMAGE_NAME_WHITE_PIXEL);

	if (myWhitePixel == nullptr)
	{
		throw Error("Failed to allocate white pixel");
	}

	if (myWhitePixel->getTexturePage() != 0)
	{
		throw Error("Failed to place white pixel on primary texture");
	}

	myWhitePixelPosition = sf::Vector2f(myWhitePixel->getTextureRect().left + 0.5f,
		myWhitePixel->getTextureRect().top + 0.5f);
}

void WOSApplication::initWindow()
{
	createInterface();
	loadIcon();
	createBackground();
	createRankCheckWidget();
	createPlayerDBProgressText();
	createSettingsPanel();
	createMenuButton();
	createInitialInfoText();
	createRootCallbacks();
}

void WOSApplication::createInterface()
{
	interface = open();
	mouseMon = gui3::make<gui3::MouseMonitor>(interface->getRootContainer());
}

void WOSApplication::loadIcon()
{
	auto icon = getResourceManager().acquireData("rankcheck/rankcheck-icon.png");
	if (icon)
	{
		sf::Image iconImg;
		if (iconImg.loadFromMemory(icon->getData(), icon->getDataSize()))
		{
			interface->setIcon(iconImg);
		}
	}
}

void WOSApplication::createBackground()
{
	fillPanel = gui3::make<gui3::FillPanel>();
	fillPanel->setZPosition(-1);
	interface->getRootContainer().add(fillPanel);

	background = gui3::make<gui3::Gradient>(getConfig().get(backgroundColor));
	background->setZPosition(-1);
	fillPanel->add(background);
}

void WOSApplication::createRankCheckWidget()
{
	rankCheck = gui3::make<RankCheckWidget>();
	fillPanel->add(rankCheck);

	rankCheck->acquireFocus();
}

void WOSApplication::createPlayerDBProgressText()
{
	playerDBProgress = gui3::make<gui3::Text>();
	playerDBProgress->setTextAlignment(0, 0);
	playerDBProgress->setRect(65, 4, 400, 0);
	interface->getRootContainer().add(playerDBProgress);
}

void WOSApplication::createSettingsPanel()
{
	settingsPanel = gui3::make<gui3::SettingsPanel>();

	addBasicMenuEntries();
	addDebugMenuEntries();

	interface->getRootContainer().add(settingsPanel);
	settingsPanel->setZPosition(1);
	settingsPanel->setRect(0, 24, 300, settingsPanel->getPreferredHeight());
	settingsPanel->setVisible(false);

	rankCheck->addMouseCallback([=](gui3::MouseEvent)
	{
		settingsPanel->setVisible(false);
	}, gui3::MouseEvent::Click);
}

void WOSApplication::createMenuButton()
{
	menuButton = gui3::make<gui3::Button>("Menu");
	menuButton->setRect(0, 0, 60, settingsPanel->getPosition().y);
	menuButton->setVisible(false);
	menuButton->addActionCallback([=]()
	{
		if (!settingsPanel->isVisible())
		{
			settingsPanel->update();
		}
		settingsPanel->setVisible(!settingsPanel->isVisible());
	});
	interface->getRootContainer().add(menuButton);
}

void WOSApplication::createInitialInfoText()
{
	initialInfoText = gui3::make<gui3::Text>("RankCheck is ready!\n\n"
		"Enter an Awesomenauts match to\n"
		"display player information.");
	initialInfoText->setTextAlignment(0, 1);
	initialInfoText->setRect(4, 4, 0, 0);
	interface->getRootContainer().add(initialInfoText);
}

void WOSApplication::createRootCallbacks()
{
	interface->getRootContainer().addTickCallback([=]()
	{
		runTickUpdates();
	});

	interface->getRootContainer().addMouseCallback([=](gui3::MouseEvent event)
	{
		handleMouseEvent(event);
	}, gui3::MouseEvent::Any);
}

void WOSApplication::addBasicMenuEntries()
{
	settingsPanel->addCheckbox(showInitialInfo, "Show initial info");

	settingsPanel->addSlider(playerPopupTimeout, [](float value)
	{
		return value < 1 ? "Auto-hide players: off" : "Auto-hide players after " + cNtoS((int)value) + "s";
	}, 0, 120);

	settingsPanel->addCheckbox(ratingHistoryVisible, "Show rating history");

	settingsPanel->addSlider(ratingHistoryTimeout, [](float value)
	{
		return value < 1 ? "Auto-hide rating: off" : "Auto-hide rating after " + cNtoS((int)value) + "s";
	}, 0, 120);

	settingsPanel->addCheckbox(ratingHistoryOnTop, "Rating history on top");

	settingsPanel->addButton([=]()
	{
		rankCheck->reshowPlayers();
		settingsPanel->setVisible(false);
	}, "Re-show current players");

	settingsPanel->addButton([=]()
	{
		rankCheck->requestPlayerDBRebuild();
		settingsPanel->setVisible(false);
	}, "Rebuild player database");

	settingsPanel->addButton([=]()
	{
		rankCheck->showGameDirChooser();
		settingsPanel->setVisible(false);
	}, "Select game directory");
}

void WOSApplication::addDebugMenuEntries()
{
#ifdef WOS_DEBUG

	settingsPanel->addButton([=]()
	{
		settingsPanel->setVisible(false);
		if (dumpAssets())
		{
			rankCheck->messageBox("Extraction successful",
				assetsFilename + " has been extracted to path '" + assetsDirectory + "'.");
		}
		else
		{
			rankCheck->messageBox("Extraction failed",
				"An error occurred while extracting " + assetsFilename + ".");
		}
	}, "Extract " + assetsFilename);

	settingsPanel->addButton([=]()
	{
		settingsPanel->setVisible(false);
		if (repackAssets())
		{
			rankCheck->messageBox("Repack successful",
				assetsFilename + " has been repacked from path '" + assetsDirectory + "'.");
		}
		else
		{
			rankCheck->messageBox("Repack failed",
				"An error occurred while repacking " + assetsFilename + ".");
		}
	}, "Repack rankcheck.dat");

#endif
}

void WOSApplication::runTickUpdates()
{
	pollConfig();
	updatePlayerDBProgress();
	updateInitialInfo();
	updateFramerate();
}

void WOSApplication::pollConfig()
{
	if (myUserConfigReloader.poll())
	{
		initConfig();
		background->setFirstColor(getConfig().get(backgroundColor));
		background->setSecondColor(getConfig().get(backgroundColor));
		interface->getRootContainer().fireStateEvent(gui3::StateEvent::RendererChanged);
		interface->getRootContainer().fireStateEvent(gui3::StateEvent::ConfigChanged);
	}
}

void WOSApplication::updatePlayerDBProgress()
{
	if (rankCheck->getPlayerDBBuildProgressVisibility() > 0.f)
	{
		playerDBProgress->setVisible(true);
		playerDBProgress->setTextColor(sf::Color(255, 255, 255, rankCheck->getPlayerDBBuildProgressVisibility() * 255));
		playerDBProgress->setText(rankCheck->getPlayerDBBuildProgress());
	}
	else
	{
		playerDBProgress->setVisible(false);
	}
}

void WOSApplication::updateInitialInfo()
{
	if (rankCheck->isInitialInfoVisible() && !rankCheck->isPlayerDBBuildRunning() && getConfig().get(showInitialInfo))
	{
		int offset = getConfig().get(ratingHistoryOnTop) ? 0 : rankCheck->getRatingBarHeight();
		initialInfoText->setPosition(4, interface->getRootContainer().getSize().y - 4 - offset);
		initialInfoText->setVisible(true);
	}
	else
	{
		initialInfoText->setVisible(false);
	}
}

void WOSApplication::updateFramerate()
{
	float fps = 60;
	if (rankCheck->isAnimating() || mouseMon->isMouseOver() || rankCheck->getPlayerDBBuildProgressVisibility() > 0.01f)
	{
		fps = getConfig().get(focusedFrameRate);
		if (fps < 0.001f)
		{
			fps = 60;
		}
	}
	else
	{
		fps = getConfig().get(unfocusedFrameRate);
		if (fps < 0.001f)
		{
			fps = 3;
		}
	}
	setFramerateLimit(fps);
}

void WOSApplication::handleMouseEvent(gui3::MouseEvent event)
{
	switch (event.type)
	{
	case gui3::MouseEvent::Enter:
	case gui3::MouseEvent::Move:
		menuButton->setVisible(true);
		break;
	case gui3::MouseEvent::Leave:
		if (!settingsPanel->isVisible())
		{
			menuButton->setVisible(false);
		}
		break;
	default:
		break;
	}
}

gui3::Interface* WOSApplication::makeInterface()
{
	return new WOSInterface(this);
}

bool WOSApplication::dumpAssets()
{
	std::unique_ptr<Package> package = makeUnique<Package>();
	if (!package->openFile(assetsFilename))
	{
		debug() << "Asset extraction failed: package file no longer exists";
		return false;
	}

	bool foundContent = package->firstContent();

	while (foundContent)
	{
		std::string contentFile = assetsDirectory + "/" + package->getContentId();
		createDirectories(removeFileName(contentFile));

		DataStream outStream;
		if (outStream.openOutFile(contentFile))
		{
			outStream.addData(package->getContentData().data(), package->getContentSize());
		}
		else
		{
			debug() << "Failed to open output stream for content file " << contentFile;
		}

		foundContent = package->nextContent();
	}

	return true;
}

bool WOSApplication::repackAssets()
{
	if (!isDirectory(assetsDirectory))
	{
		debug() << "Assets directory '" << assetsDirectory << "' is missing or not a directory.";
		return false;
	}

	if (!Package::compile(assetsDirectory, assetsFilename, StringStream::Cout))
	{
		debug() << "An error occurred during package compilation.";
		return false;
	}

	try
	{
		initAssets();
		initConfig();
	}
	catch (std::exception & ex)
	{
		debug() << "Asset recompilation resulted in invalid package: " << ex.what();
		return false;
	}

	return true;
}
