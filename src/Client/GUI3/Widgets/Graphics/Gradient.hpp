#ifndef SRC_CLIENT_GUI3_WIDGETS_GRADIENT_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_GRADIENT_HPP_

#include <Client/GUI3/Widget.hpp>
#include <SFML/Graphics/Color.hpp>

namespace gui3
{

class Gradient : public Widget
{
public:

	enum Direction
	{
		Horizontal,
		Vertical
	};

	Gradient(sf::Color color = sf::Color::Transparent);
	Gradient(Direction direction, sf::Color firstColor, sf::Color secondColor);
	virtual ~Gradient();

	void setDirection(Direction direction);
	Direction getDirection() const;

	void setFirstColor(sf::Color firstColor);
	sf::Color getFirstColor() const;

	void setSecondColor(sf::Color secondColor);
	sf::Color getSecondColor() const;

	void setOutlineThickness(float outlineThickness);
	float getOutlineThickness() const;

	void setOutlineColor(sf::Color outlineColor);
	sf::Color getOutlineColor() const;

private:

	void onRepaint(Canvas & canvas) override;

	Direction myDirection;
	sf::Color myFirstColor;
	sf::Color mySecondColor;

	float myOutlineThickness;
	sf::Color myOutlineColor;
};

}

#endif /* SRC_CLIENT_GUI3_WIDGETS_GRADIENT_HPP_ */
