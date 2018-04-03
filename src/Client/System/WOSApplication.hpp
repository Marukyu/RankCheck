#ifndef WOS_APPLICATION_HPP
#define WOS_APPLICATION_HPP

#include <Client/GUI3/Application.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Utils/MouseMonitor.hpp>
#include <Client/GUI3/Widgets/Controls/Button.hpp>
#include <Client/GUI3/Widgets/Controls/Text.hpp>
#include <Client/GUI3/Widgets/Graphics/Gradient.hpp>
#include <Client/GUI3/Widgets/Panels/FillPanel.hpp>
#include <Client/GUI3/Widgets/Panels/SettingsPanel.hpp>
#include <Client/RankCheck/RankCheckWidget.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Utils/Filesystem/DirectoryObserver.hpp>
#include <Shared/Utils/Filesystem/FileObserver.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>


// Thanks, Microsoft!
#undef interface


namespace std
{
class exception;
}

namespace cfg
{
class ConfigAggregator;
class Config;
class JSONConfig;
}

class WOSResourceManager;

namespace gui3
{
class Interface;
}

namespace sf
{
class Texture;
}

class WOSApplication : public gui3::Application
{
public:

	WOSApplication();
	virtual ~WOSApplication();

	virtual const sf::Texture * getTexture(std::size_t pageIndex) const override;

	virtual gui3::res::ResourceManager & getResourceManager() const override;

	virtual sf::Vector2f getWhitePixel() const override;

	virtual cfg::Config & getConfig() const override;

	void saveConfig();

private:

	enum LoadReturnCodes
	{
		LoadSuccess = 0,
		LoadError
	};

	virtual int init(const std::vector<std::string> & args) override;

	void initCrashHandler();
	void initAssets();
	void initConfig();
	void initWhitePixel();
	void initWindow();

	void createInterface();
	void loadIcon();
	void createBackground();
	void createRankCheckWidget();
	void createPlayerDBProgressText();
	void createSettingsPanel();
	void createMenuButton();
	void createInitialInfoText();
	void createRootCallbacks();

	void addBasicMenuEntries();
	void addDebugMenuEntries();

	void runTickUpdates();

	void pollConfig();
	void updatePlayerDBProgress();
	void updateInitialInfo();
	void updateFramerate();

	void handleMouseEvent(gui3::MouseEvent event);

	bool dumpAssets();
	bool repackAssets();

	virtual void handleError(std::exception & exception) override;
	void displayError(std::exception & exception, std::string messageTitle = "");

	virtual gui3::Interface * makeInterface() override;

	std::unique_ptr<WOSResourceManager> myResourceManager;

	std::unique_ptr<cfg::Config> myConfig;
	std::shared_ptr<cfg::ConfigAggregator> myConfigAggregator;
	std::shared_ptr<cfg::JSONConfig> myBuiltinConfig;
	std::shared_ptr<cfg::JSONConfig> myUserConfig;
	std::shared_ptr<cfg::JSONConfig> myApplicationConfig;

	gui3::Ptr<gui3::res::Image> myWhitePixel;
	sf::Vector2f myWhitePixelPosition;

	fs::FileObserver myBuiltinConfigReloader;
	fs::FileObserver myUserConfigReloader;

	fs::DirectoryObserver myReplaysFolderObserver;
	fs::FileObserver myReplayInfoFileObserver;

	gui3::Interface * interface = nullptr;
	gui3::Ptr<gui3::MouseMonitor> mouseMon;
	gui3::Ptr<gui3::FillPanel> fillPanel;
	gui3::Ptr<gui3::Gradient> background;
	gui3::Ptr<gui3::Text> playerDBProgress;
	gui3::Ptr<gui3::Text> initialInfoText;
	gui3::Ptr<gui3::Button> menuButton;
	gui3::Ptr<gui3::SettingsPanel> settingsPanel;
	gui3::Ptr<RankCheckWidget> rankCheck;

	std::string applicationConfigFilename;
};

#endif
