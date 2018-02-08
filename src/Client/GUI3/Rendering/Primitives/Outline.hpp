#ifndef SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_OUTLINE_HPP_
#define SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_OUTLINE_HPP_

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace gui3
{
namespace primitives
{

class Outline
{
public:

	enum Sides
	{
		Left = 1 << 0,
		Top = 1 << 1,
		Right = 1 << 2,
		Bottom = 1 << 3,

		All = Left | Top | Right | Bottom
	};

	Outline(sf::FloatRect rect, float thickness, sf::Color color, int sides = All);

	const sf::FloatRect & getRect() const;
	float getThickness() const;
	sf::Color getColor() const;

	bool testSide(Sides side) const;

private:

	sf::FloatRect myRect;
	float myThickness;
	sf::Color myColor;
	int mySides;
};

}
}

#endif
