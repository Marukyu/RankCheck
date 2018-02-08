/*
 * MouseEvent.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_EVENTS_MOUSEEVENT_HPP_
#define SRC_CLIENT_GUI3_EVENTS_MOUSEEVENT_HPP_

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cassert>
#include <cstddef>

namespace gui3
{

class MouseEvent
{
public:

	typedef float ScrollOffset;

	enum Button
		: unsigned int
		{
			Left = 0,
		Right,
		Middle,
		Extra1,
		Extra2,

		UnknownButton,
		ButtonCount
	};

	enum Type
	{
		/**
		 * Empty event mask.
		 */
		None = 0,

		/** 
		 * Called whenever a mouse button that was previously pressed on the widget is released on the widget.
		 */
		Click = 1 << 0,

		/** 
		 * Called whenever a mouse button is pressed on the widget.
		 */
		ButtonDown = 1 << 1,

		/** 
		 * Called whenever a mouse button that was previously pressed on the widget is released anywhere.
		 */
		ButtonUp = 1 << 2,

		/** 
		 * Called whenever the mouse cursor's position changes while the mouse cursor is on the widget.
		 */
		Move = 1 << 3,

		/** 
		 * Called whenever the mouse cursor's position changes to being on the widget after having been off the widget.
		 */
		Enter = 1 << 4,

		/** 
		 * Called whenever the mouse cursor's position changes to being off the widget after having been on the widget.
		 */
		Leave = 1 << 5,

		/** 
		 * Called whenever the mouse wheel is rotated horizontally.
		 */
		ScrollX = 1 << 6,

		/** 
		 * Called whenever the mouse wheel is rotated vertically.
		 */
		ScrollY = 1 << 7,

		/**
		 * Full event mask (for event forwarding).
		 */
		Any = 0x7fffffff
	};

	static MouseEvent generatePositionalEvent(Type type, sf::Vector2f position)
	{
		assert(type == Move || type == Enter || type == Leave);
		return MouseEvent(type, position);
	}

	static MouseEvent generateButtonEvent(Type type, sf::Vector2f position, Button button)
	{
		assert(type == Click || type == ButtonDown || type == ButtonUp);
		assert(button < ButtonCount);
		return MouseEvent(type, position, button);
	}

	static MouseEvent generateScrollEvent(Type type, sf::Vector2f position, ScrollOffset scrollAmount)
	{
		assert(type == ScrollX || type == ScrollY);
		return MouseEvent(type, position, scrollAmount);
	}

	static MouseEvent generateRepositionedEvent(MouseEvent event, sf::Vector2f position)
	{
		switch (event.type)
		{
		case None:
		case Move:
		case Enter:
		case Leave:
		default:
			return MouseEvent(event.type, position);

		case Click:
		case ButtonDown:
		case ButtonUp:
			return MouseEvent(event.type, position, event.button);

		case ScrollX:
		case ScrollY:
			return MouseEvent(event.type, position, event.scrollAmount);
		}
	}

	static Button getButtonConstantFromSFML(sf::Mouse::Button button)
	{
		switch (button)
		{
		case sf::Mouse::Left:
			return Left;
		case sf::Mouse::Right:
			return Right;
		case sf::Mouse::Middle:
			return Middle;
		case sf::Mouse::XButton1:
			return Extra1;
		case sf::Mouse::XButton2:
			return Extra2;
		default:
			return UnknownButton;
		}
	}

	static Type getScrollTypeConstantFromSFML(sf::Mouse::Wheel wheel)
	{
		switch (wheel)
		{
		case sf::Mouse::Wheel::HorizontalWheel:
			return ScrollX;
		case sf::Mouse::Wheel::VerticalWheel:
			return ScrollY;
		default:
			return None;
		}
	}

	const Type type;

	const sf::Vector2f position;

	union
	{
		const Button button;
		const ScrollOffset scrollAmount;
	};

private:

	MouseEvent(Type type, sf::Vector2f position) :
		type(type),
		position(position),
		button(static_cast<Button>(0))
	{
	}

	MouseEvent(Type type, sf::Vector2f position, Button button) :
		type(type),
		position(position),
		button(button)
	{
	}

	// Use dummy parameter to specify scroll event type.
	MouseEvent(Type type, sf::Vector2f position, ScrollOffset scrollAmount) :
		type(type),
		position(position),
		scrollAmount(scrollAmount)
	{
	}
};

}

#endif
