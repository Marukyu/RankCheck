#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_CHECKBOX_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_CHECKBOX_HPP_

#include <Client/GUI3/Widget.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <string>


namespace gui3
{

namespace pieces
{
class Button;
class Text;
}

/**
 * 2-state checkbox with a label.
 */
class Checkbox : public Widget
{
public:

	/**
	 * Event class for checkbox events.
	 */
	class Event
	{
	public:

		enum Type
		{
			/**
			 * Fired when the value of the checkbox changes, either by the user or through the "setChecked" function.
			 */
			Changed = 1 << 0,

			/**
			 * Fired when the user clicks on the checkbox, toggling its value.
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

	/**
	 * Default constructor. Creates an unchecked checkbox.
	 */
	Checkbox();

	/**
	 * Constructor with specified label and initial checkbox value.
	 */
	Checkbox(std::string text, bool checkboxValue = false);

	/**
	 * Destructor.
	 */
	virtual ~Checkbox();

	/**
	 * Changes the checkbox's label.
	 */
	void setText(std::string text);

	/**
	 * Returns the checkbox's label.
	 */
	const std::string & getText() const;

	/**
	 * Toggles the checkbox's check state.
	 */
	void toggle();

	/**
	 * Checks/unchecks the checkbox depending on the specified argument.
	 */
	void setChecked(bool checked);

	/**
	 * Returns true if the checkbox is currently checked, false otherwise.
	 */
	bool isChecked() const;

	/**
	 * Registers a function to be called whenever the checkbox is toggled.
	 */
	CallbackHandle<Event> addEventCallback(EventFunc<Event> func, int typeFilter, int order = 0);

	/**
	 * Calls all registered functions without changing the checkbox's current state.
	 */
	void fireEvent(Event event);

private:

	virtual void onRepaint(Canvas & canvas) override;

	void updateButtonFlags();
	void updateConfigValues();

	pieces::Button * myButton;
	pieces::Text * myText;

	bool myIsChecked;

	CallbackManager<Event> myEventCallbacks;
};

}

#endif
