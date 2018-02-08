#ifndef SRC_CLIENT_GUI3_UTILS_MOUSEMONITOR_HPP_
#define SRC_CLIENT_GUI3_UTILS_MOUSEMONITOR_HPP_

#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Widget.hpp>
#include <SFML/System/Vector2.hpp>
#include <bitset>

namespace gui3
{

class MouseMonitor
{
public:
	MouseMonitor(Widget & widget, int order = -50);
	virtual ~MouseMonitor();

	bool isMouseOver() const;
	bool isMouseDown(MouseEvent::Button button = MouseEvent::Left) const;
	bool isMouseDownAnyButton() const;
	sf::Vector2f getMousePosition() const;

private:

	void handleMouseEvent(MouseEvent event);

	/**
	 * Internal representation of the current mouse state.
	 * 
	 * 0th bit is mouse over state. Subsequent bits are the pressed state of each mouse button.
	 */
	std::bitset<MouseEvent::ButtonCount + 1> myBitset;

	sf::Vector2f myMousePosition;

	Callback<MouseEvent> myCallback;
};

}

#endif
