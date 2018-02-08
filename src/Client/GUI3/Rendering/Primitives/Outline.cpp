#include <Client/GUI3/Rendering/Primitives/Outline.hpp>

namespace gui3
{
namespace primitives
{

Outline::Outline(sf::FloatRect rect, float thickness, sf::Color color, int sides) :
	myRect(rect),
	myThickness(thickness),
	myColor(color),
	mySides(sides)
{
}

const sf::FloatRect& Outline::getRect() const
{
	return myRect;
}

float Outline::getThickness() const
{
	return myThickness;
}

sf::Color Outline::getColor() const
{
	return myColor;
}

bool Outline::testSide(Sides side) const
{
	return mySides & side;
}

}
}
