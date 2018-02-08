/*
 * Renderer.cpp
 *
 *  Created on: Jul 10, 2015
 *      Author: marukyu
 */

#include <Client/GUI3/Renderer.hpp>
#include <Client/GUI3/Rendering/Primitives/Gradient.hpp>
#include <Client/GUI3/Rendering/Primitives/Outline.hpp>
#include <Client/GUI3/Rendering/Primitives/TintedBox.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <array>

namespace gui3
{

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

sf::Vector2f Renderer::getWhitePixel() const
{
	return sf::Vector2f(0, 0);
}

void Renderer::addPrimitive(Canvas& canvas, primitives::Gradient gradient) const
{
	std::array<sf::Vertex, 6> vertices;

	sf::FloatRect rect = gradient.getRect();

	sf::Color colorTL = gradient.getFirstColor();
	sf::Color colorBR = gradient.getSecondColor();
	sf::Color colorTR = gradient.getDirection() == primitives::Gradient::Horizontal ? colorBR : colorTL;
	sf::Color colorBL = gradient.getDirection() == primitives::Gradient::Horizontal ? colorTL : colorBR;

	vertices[0] = sf::Vertex(sf::Vector2f(rect.left, rect.top), colorTL);
	vertices[1] = sf::Vertex(sf::Vector2f(rect.left + rect.width, rect.top), colorTR);
	vertices[2] = sf::Vertex(sf::Vector2f(rect.left, rect.top + rect.height), colorBL);
	vertices[3] = vertices[2];
	vertices[4] = vertices[1];
	vertices[5] = sf::Vertex(sf::Vector2f(rect.left + rect.width, rect.top + rect.height), colorBR);

	canvas.draw(vertices.begin(), vertices.end());
}

void Renderer::addPrimitive(Canvas& canvas, primitives::Outline outline) const
{
	sf::FloatRect rect = outline.getRect();
	float thickness = -outline.getThickness();
	sf::Color color = outline.getColor();

	// Top and bottom are inner outlines.
	if (outline.testSide(primitives::Outline::Top))
	{
		sf::FloatRect outlineRectTop(rect.left, -thickness + rect.top, rect.width, thickness);
		addPrimitive(canvas, primitives::Gradient(outlineRectTop, color));
	}

	if (outline.testSide(primitives::Outline::Bottom))
	{
		sf::FloatRect outlineRectBottom(rect.left, rect.height + rect.top, rect.width, thickness);
		addPrimitive(canvas, primitives::Gradient(outlineRectBottom, color));
	}

	// Left and right are outer outlines (they cover the corners too).
	if (outline.testSide(primitives::Outline::Left))
	{
		sf::FloatRect outlineRectLeft(-thickness + rect.left, -thickness + rect.top, thickness,
			rect.height + thickness * 2.f);
		addPrimitive(canvas, primitives::Gradient(outlineRectLeft, color));
	}

	if (outline.testSide(primitives::Outline::Right))
	{
		sf::FloatRect outlineRectRight(rect.width + rect.left, -thickness + rect.top, thickness,
			rect.height + thickness * 2.f);
		addPrimitive(canvas, primitives::Gradient(outlineRectRight, color));
	}
}

void Renderer::addPrimitive(Canvas& canvas, primitives::Box box) const
{
	addPrimitive(canvas,
		primitives::Gradient(box.getRect(), primitives::Gradient::Vertical, sf::Color::White, sf::Color::Black));
}

void Renderer::addPrimitive(Canvas& canvas, primitives::TintedBox tintedBox) const
{
	addPrimitive(canvas, primitives::Gradient(tintedBox.getRect(), tintedBox.getTintColor()));
}

}
