#ifndef SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_BOX_HPP_
#define SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_BOX_HPP_

#include <SFML/Graphics/Rect.hpp>

namespace gui3
{
namespace primitives
{

class Box
{
public:

	enum Flags
	{
		// Default, button-like appearance.
		Normal = 0,

		// Mouse is over the widget.
		Hover = 1 << 0,

		// Mouse is pressed on the widget.
		Pressed = 1 << 1,

		// Widget interaction is disabled.
		Disabled = 1 << 2,

		// Used for panel visuals (transparent background rectangle).
		Background = 1 << 3,

		// Toggleable widget is in "on" state (checkbox, radiobox, toggle button).
		Marked = 1 << 4,

		// Widget has keyboard focus.
		Focused = 1 << 5,

		// Darkened background (text field, list).
		Dark = 1 << 6,

		// Selection box (text field, list).
		Selection = 1 << 7,

		// All flags combined.
		AllFlags = (1 << 8) - 1
	};

	enum Shape
	{
		Rectangle,
		Circle
	};

	Box(sf::FloatRect rect, int flags = Normal, Shape shape = Rectangle);

	sf::FloatRect getRect() const;
	bool testFlag(Flags flag) const;
	Shape getShape() const;

private:

	sf::FloatRect myRect;
	int myFlags;
	Shape myShape;
};

}
}

#endif
