#ifndef SRC_CLIENT_SYSTEM_WOSRENDERER_HPP_
#define SRC_CLIENT_SYSTEM_WOSRENDERER_HPP_

#include <Client/GUI3/Renderer.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <SFML/System/Vector2.hpp>

namespace cfg
{
class Config;
}
namespace gui3
{
class Canvas;
}

/**
 * Basic renderer with hardcoded theme.
 */
class WOSRenderer : public gui3::Renderer
{
public:
	WOSRenderer();
	virtual ~WOSRenderer();

	void setConfig(cfg::Config * config);
	cfg::Config & getConfig() const;

	void setWhitePixel(sf::Vector2f whitePixel);
	virtual sf::Vector2f getWhitePixel() const override;

	virtual void addPrimitive(gui3::Canvas & canvas, gui3::primitives::Gradient gradient) const override;
	virtual void addPrimitive(gui3::Canvas & canvas, gui3::primitives::Outline outline) const override;
	virtual void addPrimitive(gui3::Canvas & canvas, gui3::primitives::Box box) const override;
	virtual void addPrimitive(gui3::Canvas & canvas, gui3::primitives::TintedBox tintedBox) const override;

private:

	cfg::Config * myConfig;
	sf::Vector2f myWhitePixel;
};

#endif
