#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_SLIDER_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_SLIDER_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Utils/MouseMonitor.hpp>
#include <Client/GUI3/Widget.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>

namespace gui3
{
namespace pieces
{
class Button;
}

class Slider : public Widget
{
public:

	/**
	 * Event class for slider events.
	 */
	class Event
	{
	public:

		enum Type
		{
			/**
			 * Fired when the value of the slider changes, either by the user or through the "setValue" function.
			 */
			Changed = 1 << 0,

			/**
			 * Fired when the user drags the slider, changing its value.
			 */
			UserChanged = 1 << 1,

			/**
			 * Full event mask.
			 */
			Any = 0x7fffffff
		};

		Event(Type type) :
			type(type)
		{
		}

		const Type type;
	};

	enum Direction
	{
		Auto,
		Horizontal,
		Vertical
	};

	Slider();
	virtual ~Slider();

	void setMinimumValue(float minimum);
	void setMaximumValue(float maximum);
	void setBounds(float minimum, float maximum);

	float getMinimumValue() const;
	float getMaximumValue() const;

	void setValue(float value);
	float getValue() const;

	void setDirection(Direction direction);
	Direction getDirection() const;
	Direction getCurrentDirection() const;

	void setScrollSpeed(float scrollSpeed);
	float getScrollSpeed() const;

	void setSmoothScrollingEnabled(bool smooth);
	bool isSmoothScrollingEnabled() const;

	/**
	 * Registers a function to be called whenever the slider value is changed.
	 */
	CallbackHandle<Event> addEventCallback(EventFunc<Event> func, int typeFilter, int order = 0);

	/**
	 * Calls all registered functions without changing the slider's value.
	 */
	void fireEvent(Event event);

	/**
	 * Listens for mouse wheel scroll events on another widget, passing the events to this slider.
	 */
	CallbackHandle<MouseEvent> linkWithWidget(Widget & widget, int order = -50);

private:

	void onRepaint(Canvas & canvas) override;

	void handleStateEvent(StateEvent event);
	void handleMouseEvent(MouseEvent event);
	void handleRemoteMouseEvent(MouseEvent event);

	void scrollRelative(float amount);
	void scrollToMousePosition(sf::Vector2f mousePos);
	void setValuePreserveMomentum(float value);

	void setTargetValue(float value);
	float getTargetValue() const;

	float clampValue(float value) const;

	void updateConfig();
	void updateHandle();

	void onTick();

	pieces::Button * myHandle;
	MouseMonitor myMouseMonitor;

	CallbackManager<Event> myEventCallbacks;

	float myValue;
	float myTargetValue;
	float myMinimumValue;
	float myMaximumValue;
	Direction myDirection;
	float myScrollSpeed;
	bool mySmoothScroll;

	sf::Clock myScrollTimer;
};

}

#endif
