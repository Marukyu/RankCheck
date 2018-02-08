#include <Client/GUI3/Rendering/Primitives/Gradient.hpp>
#include <Client/GUI3/Rendering/Primitives/Outline.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widgets/Graphics/Gradient.hpp>

namespace gui3
{

Gradient::Gradient(sf::Color color) :
	myDirection(Horizontal),
	myFirstColor(color),
	mySecondColor(color),
	myOutlineThickness(0),
	myOutlineColor(sf::Color::Black)
{
}

Gradient::Gradient(Direction direction, sf::Color firstColor, sf::Color secondColor) :
	myDirection(direction),
	myFirstColor(firstColor),
	mySecondColor(secondColor),
	myOutlineThickness(0),
	myOutlineColor(sf::Color::Black)
{
}

Gradient::~Gradient()
{
}

void Gradient::onRepaint(Canvas& canvas)
{
	sf::FloatRect rect = getBaseRect();

	// Check if outline needs to be drawn. Float comparison to 0 is ok because the value is set accurately by the user.
	if (getOutlineThickness() != 0)
	{
		// Calculate outline.
		primitives::Outline outline(rect, getOutlineThickness(), getOutlineColor());

		// Draw outline.
		canvas.draw(outline);

		// Shrink rectangle if needed.
		if (getOutlineThickness() > 0.f)
		{
			rect.left += getOutlineThickness();
			rect.top += getOutlineThickness();
			rect.width -= getOutlineThickness() * 2.f;
			rect.height -= getOutlineThickness() * 2.f;
		}
	}

	// Calculate gradient direction.
	primitives::Gradient::Direction dir =
		getDirection() == Horizontal ? primitives::Gradient::Horizontal : primitives::Gradient::Vertical;

	// Draw gradient.
	canvas.draw(primitives::Gradient(rect, dir, getFirstColor(), getSecondColor()));
}

void Gradient::setDirection(Direction direction)
{
	if (myDirection != direction)
	{
		myDirection = direction;
		repaint();
	}
}

Gradient::Direction Gradient::getDirection() const
{
	return myDirection;
}

void Gradient::setFirstColor(sf::Color firstColor)
{
	if (myFirstColor != firstColor)
	{
		myFirstColor = firstColor;
		repaint();
	}
}

sf::Color Gradient::getFirstColor() const
{
	return myFirstColor;
}

void Gradient::setSecondColor(sf::Color secondColor)
{
	if (mySecondColor != secondColor)
	{
		mySecondColor = secondColor;
		repaint();
	}
}

sf::Color Gradient::getSecondColor() const
{
	return mySecondColor;
}

void Gradient::setOutlineThickness(float outlineThickness)
{
	if (myOutlineThickness != outlineThickness)
	{
		myOutlineThickness = outlineThickness;
		repaint();
	}
}

float Gradient::getOutlineThickness() const
{
	return myOutlineThickness;
}

void Gradient::setOutlineColor(sf::Color outlineColor)
{
	if (myOutlineColor != outlineColor)
	{
		myOutlineColor = outlineColor;
		repaint();
	}
}

sf::Color Gradient::getOutlineColor() const
{
	return myOutlineColor;
}

}
