#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_BUTTON_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_BUTTON_HPP_

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

class Canvas;

/**
 * Button without a state.
 */
class Button : public Widget
{
public:

	/**
	 * Default constructor: creates a button with an empty label.
	 */
	Button();

	/**
	 * Constructor: creates a button with the specified label.
	 */
	Button(std::string text);

	/**
	 * Destructor.
	 */
	virtual ~Button();

	/**
	 * Changes the button's label.
	 */
	void setText(std::string text);

	/**
	 * Returns the button's label.
	 */
	const std::string & getText() const;

	/**
	 * Registers a function to be called whenever the button is pressed.
	 */
	CallbackHandle<> addActionCallback(EventFunc<> func, int order = 0);

	/**
	 * Presses the button, calling all registered functions.
	 */
	void fireAction();

private:

	void onRepaint(Canvas & canvas) override;

	pieces::Button * myButton;
	pieces::Text * myText;

	CallbackManager<> myActionCallbacks;
};

}

#endif
