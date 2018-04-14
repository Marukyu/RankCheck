#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <Client/System/WOSApplication.hpp>
#include <Client/System/WOSInterface.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>

static cfg::Vector2f windowSizeCfg("rankcheck.initWindowSize");

WOSInterface::WOSInterface(WOSApplication * parentApplication) :
	gui3::Interface(parentApplication)
{
	setTitle("RankCheck v1.5.1");

	myRenderer.setWhitePixel(parentApplication->getWhitePixel());
	myRenderer.setConfig(&parentApplication->getConfig());
	getRootContainer().setRendererOverride(&myRenderer);

	sf::Vector2i windowSize(parentApplication->getConfig().get(windowSizeCfg));

	if (windowSize.x <= 0 || windowSize.y <= 0)
	{
		windowSize = {1280, 720};
	}

	resize(sf::Vector2u(windowSize), false);
}

WOSInterface::~WOSInterface()
{
	getParentApplication().getConfig().set(windowSizeCfg, sf::Vector2f(getSize()));
}
