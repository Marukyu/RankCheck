#include <Client/Graphics/FloatColor.hpp>
#include <Client/GUI3/Rendering/Primitives/Gradient.hpp>
#include <Client/GUI3/Rendering/Primitives/Outline.hpp>
#include <Client/GUI3/Rendering/Primitives/TintedBox.hpp>
#include <Client/System/WOSRenderer.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using gui3::primitives::Box;
using gui3::primitives::TintedBox;
using gui3::primitives::Gradient;
using gui3::primitives::Outline;

WOSRenderer::WOSRenderer()
{
}

WOSRenderer::~WOSRenderer()
{
}

void WOSRenderer::setConfig(cfg::Config * config)
{
	myConfig = config;
}

cfg::Config & WOSRenderer::getConfig() const
{
	return *myConfig;
}

void WOSRenderer::setWhitePixel(sf::Vector2f whitePixel)
{
	myWhitePixel = whitePixel;
}

sf::Vector2f WOSRenderer::getWhitePixel() const
{
	return myWhitePixel;
}

void WOSRenderer::addPrimitive(gui3::Canvas& canvas, Gradient gradient) const
{
	// Superclass already implements this properly.
	Renderer::addPrimitive(canvas, gradient);
}

void WOSRenderer::addPrimitive(gui3::Canvas& canvas, Outline outline) const
{
	// Superclass already implements this properly.
	Renderer::addPrimitive(canvas, outline);
}

void WOSRenderer::addPrimitive(gui3::Canvas& canvas, Box box) const
{
	static cfg::Color configBoxColor("wos.gui.renderer.box.baseColor");
	// sf::Color(96, 96, 96)
	addPrimitive(canvas, TintedBox(box, getConfig().get(configBoxColor)));
}

namespace
{
struct BoxSetting
{
	BoxSetting(Box::Flags flag, std::string key) :
		flag(flag),
		color("wos.gui.renderer.box." + key + ".color"),
		interpolation("wos.gui.renderer.box." + key + ".interpolation"),
		gradientChange("wos.gui.renderer.box." + key + ".gradientChange"),
		outlineColor("wos.gui.renderer.box." + key + ".outlineColor"),
		outlineInterpolation("wos.gui.renderer.box." + key + ".outlineInterpolation"),
		fade("wos.gui.renderer.box." + key + ".fade")
	{
	}

	Box::Flags flag;
	cfg::Color color;
	cfg::Float interpolation;
	cfg::Float gradientChange;
	cfg::Color outlineColor;
	cfg::Float outlineInterpolation;
	cfg::Float fade;
};

std::vector<BoxSetting> initBoxSettings()
{
	std::vector<std::pair<Box::Flags, std::string>> flags = {
		{ Box::Marked, "marked" },
		{ Box::Selection, "selection" },
		{ Box::Hover, "hover" },
		{ Box::Focused, "focused" },
		{ Box::Background, "background" },
		{ Box::Dark, "dark" },
		{ Box::Disabled, "disabled" } };

	std::vector<BoxSetting> settings;
	settings.reserve(flags.size());

	for (auto flag : flags)
	{
		settings.emplace_back(flag.first, flag.second);
	}

	return std::move(settings);
}

}

void WOSRenderer::addPrimitive(gui3::Canvas& canvas, TintedBox box) const
{
	static cfg::Color configOutlineColor("wos.gui.renderer.box.outlineColor");
	static cfg::Float configOutlineThickness("wos.gui.renderer.box.outlineThickness");
	static cfg::Float configGradient("wos.gui.renderer.box.baseGradient");

	static std::vector<BoxSetting> boxSettings = initBoxSettings();

	FloatColor boxOutline = getConfig().get(configOutlineColor);
	FloatColor boxFill = box.getTintColor();
	float outlineThickness = getConfig().get(configOutlineThickness);
	float alpha = 1.f;
	float gradient = getConfig().get(configGradient);
	sf::FloatRect rect = box.getRect();

	for (const auto & setting : boxSettings)
	{
		if (box.testFlag(setting.flag))
		{
			// Get fill color modifier.
			FloatColor fillMod = getConfig().get(setting.color);

			// Ignore alpha channel.
			fillMod.a = boxFill.a;

			// Apply fill color modifier.
			boxFill = FloatColor::interpolate(boxFill, fillMod, getConfig().get(setting.interpolation));

			// Apply gradient modifier.
			gradient += getConfig().get(setting.gradientChange);

			// Get outline color modifier.
			FloatColor outlineMod = getConfig().get(setting.outlineColor);

			// Ignore alpha channel.
			outlineMod.a = boxOutline.a;

			// Apply outline color modifier.
			boxOutline = FloatColor::interpolate(boxOutline, outlineMod, getConfig().get(setting.outlineInterpolation));

			// Apply alpha fade value.
			alpha *= (1.f - getConfig().get(setting.fade));
		}
	}

	// Apply alpha.
	boxFill.a *= alpha;
	boxOutline.a *= alpha;

	// Define lightening/darkening colors and interpolation amount.
	FloatColor topFadeColor(1.f, 1.f, 1.f, boxFill.a);
	FloatColor bottomFadeColor(0.f, 0.f, 0.f, boxFill.a);

	// Compute color gradient.
	FloatColor topColor(FloatColor::interpolate(boxFill, topFadeColor, gradient));
	FloatColor bottomColor(FloatColor::interpolate(boxFill, bottomFadeColor, gradient));

	// Swap colors if needed.
	if (box.testFlag(Box::Pressed))
	{
		std::swap(topColor, bottomColor);
	}

	// Check if outline needs to be drawn.
	if (outlineThickness != 0.f && ((sf::Color) boxOutline).a != 0)
	{
		addPrimitive(canvas, Outline(rect, outlineThickness, boxOutline));
	}

	// Check if filling needs to be drawn.
	if (((sf::Color) boxFill).a != 0)
	{
		// Shrink rectangle if needed.
		if (outlineThickness > 0.f)
		{
			// TODO: respect outline side flags.
			rect.left += outlineThickness;
			rect.top += outlineThickness;
			rect.width -= outlineThickness * 2.f;
			rect.height -= outlineThickness * 2.f;
		}

		// Draw filling.
		addPrimitive(canvas, Gradient(rect, Gradient::Vertical, topColor, bottomColor));
	}

}

