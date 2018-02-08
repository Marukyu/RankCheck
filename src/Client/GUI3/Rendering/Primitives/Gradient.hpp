#ifndef SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_GRADIENT_HPP_
#define SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_GRADIENT_HPP_

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace sf
{
class Color;
}

namespace gui3
{
namespace primitives
{

class Gradient
{
public:

	enum Direction
	{
		Horizontal,
		Vertical
	};

	Gradient(sf::FloatRect rect, Direction direction, sf::Color firstColor, sf::Color secondColor);
	Gradient(sf::FloatRect rect, sf::Color color);

	const sf::FloatRect& getRect() const;
	Direction getDirection() const;
	sf::Color getFirstColor() const;
	sf::Color getSecondColor() const;

private:

	sf::FloatRect myRect;
	Direction myDirection;
	sf::Color myFirstColor;
	sf::Color mySecondColor;
};

}
}

#endif
