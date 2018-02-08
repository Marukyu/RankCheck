#include <Client/GUI3/Rendering/Primitives/Gradient.hpp>

namespace gui3
{
namespace primitives
{

Gradient::Gradient(sf::FloatRect rect, Direction direction, sf::Color firstColor, sf::Color secondColor) :
	myRect(rect),
	myDirection(direction),
	myFirstColor(firstColor),
	mySecondColor(secondColor)
{
}

Gradient::Gradient(sf::FloatRect rect, sf::Color color) :
	myRect(rect),
	myDirection(Direction::Horizontal),
	myFirstColor(color),
	mySecondColor(color)
{
}

const sf::FloatRect& Gradient::getRect() const
{
	return myRect;
}

Gradient::Direction Gradient::getDirection() const
{
	return myDirection;
}

sf::Color Gradient::getFirstColor() const
{
	return myFirstColor;
}

sf::Color Gradient::getSecondColor() const
{
	return mySecondColor;
}

}
}

