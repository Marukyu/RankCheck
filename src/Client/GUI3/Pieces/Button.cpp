#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Pieces/Button.hpp>
#include <Client/GUI3/Rendering/Primitives/TintedBox.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widget.hpp>

namespace gui3
{

namespace pieces
{

Button::Button() :
	myBoxFlags(primitives::Box::Normal),
	myOnBoxFlags(primitives::Box::Normal),
	myOffBoxFlags(primitives::Box::Normal),
	myIsColorSet(false)
{
	setAutoResizeFunction();
}

Button::~Button()
{
}

void Button::paint(Canvas& canvas)
{
	int flagsEnabled = myBoxFlags;
	int flagsDisabled = primitives::Box::Disabled;
	int flags = getWidget()->isEnabled() ? flagsEnabled : flagsDisabled;

	flags |= myOnBoxFlags;
	flags &= ~myOffBoxFlags;

	primitives::Box box(getRect(), (primitives::Box::Flags) flags);

	if (myIsColorSet)
	{
		canvas.draw(primitives::TintedBox(box, myColor));
	}
	else
	{
		canvas.draw(box);
	}
}

void Button::setFlagsOn(primitives::Box::Flags flags)
{
	myOnBoxFlags |= flags;
	myOffBoxFlags &= ~flags;
	repaint();
}

void Button::setFlagsOff(primitives::Box::Flags flags)
{
	myOffBoxFlags |= flags;
	myOnBoxFlags &= ~flags;
	repaint();
}

void Button::resetFlags(primitives::Box::Flags flags)
{
	myOnBoxFlags &= ~flags;
	myOffBoxFlags &= ~flags;
	repaint();
}

void Button::setColor(sf::Color color)
{
	myIsColorSet = true;
	myColor = color;
	repaint();
}

void Button::resetColor()
{
	myIsColorSet = false;
}

sf::Color Button::getColor() const
{
	return myIsColorSet ? myColor : sf::Color::Transparent;
}

bool Button::isColorSet() const
{
	return myIsColorSet;
}

void Button::handleMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::Enter:
		myBoxFlags |= primitives::Box::Hover;
		break;

	case MouseEvent::Leave:
		myBoxFlags &= ~primitives::Box::Hover;
		break;

	case MouseEvent::ButtonDown:
		myBoxFlags |= primitives::Box::Pressed;
		break;

	case MouseEvent::ButtonUp:
		myBoxFlags &= ~primitives::Box::Pressed;
		break;

	default:
		return;
	}

	repaint();
}

void Button::onInit()
{
	getWidget()->addMouseCallback([this](MouseEvent event)
	{
		handleMouseEvent(event);
	}, MouseEvent::Any);
	getWidget()->addStateCallback([this](StateEvent event)
	{
		handleStateEvent(event);
	}, StateEvent::Any);
}

void Button::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::FocusGained:
		myBoxFlags |= primitives::Box::Focused;
		break;

	case StateEvent::FocusLost:
		myBoxFlags &= ~primitives::Box::Focused;
		break;

	case StateEvent::UsabilityChanged:
		// Enabled/disabled distinction is handled in the paint function.
		break;

	default:
		return;
	}

	repaint();
}

}
}
