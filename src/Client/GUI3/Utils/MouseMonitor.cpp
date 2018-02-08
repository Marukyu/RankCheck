#include <Client/GUI3/Utils/MouseMonitor.hpp>

namespace gui3
{

MouseMonitor::MouseMonitor(Widget& widget, int order)
{
	myCallback = widget.addMouseCallback([this](MouseEvent event)
	{
		handleMouseEvent(event);
	}, MouseEvent::Any, order);
}

MouseMonitor::~MouseMonitor()
{
}

bool MouseMonitor::isMouseOver() const
{
	return myBitset.test(0);
}

bool MouseMonitor::isMouseDown(MouseEvent::Button button) const
{
	return myBitset.test(button + 1);
}

bool MouseMonitor::isMouseDownAnyButton() const
{
	for (typename std::underlying_type<MouseEvent::Button>::type button = MouseEvent::Left;
		button < MouseEvent::ButtonCount; ++button)
	{
		if (isMouseDown((MouseEvent::Button) button))
		{
			return true;
		}
	}
	return false;
}

sf::Vector2f MouseMonitor::getMousePosition() const
{
	return myMousePosition;
}

void MouseMonitor::handleMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ButtonDown:
		myBitset.set(event.button + 1);
		break;

	case MouseEvent::ButtonUp:
		myBitset.reset(event.button + 1);
		break;

	case MouseEvent::Enter:
		myBitset.set(0);
		break;

	case MouseEvent::Leave:
		myBitset.reset(0);
		break;

	case MouseEvent::Move:
		myBitset.set(0);
		myMousePosition = event.position;
		break;

	default:
		break;
	}
}

}
