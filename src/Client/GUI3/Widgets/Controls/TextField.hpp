#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_TEXTFIELD_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_TEXTFIELD_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Utils/MouseMonitor.hpp>
#include <Client/GUI3/Widget.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <Shared/Utils/Range.hpp>
#include <cstddef>
#include <string>

namespace gui3
{
namespace pieces
{
class Text;
}

class TextField : public Widget
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
			 * Fired when the text of the text field changes, either by the user or through the "setText" function.
			 */
			Changed = 1 << 0,

			/**
			 * Fired when the user modifies the text inside the text field.
			 */
			UserChanged = 1 << 1,

			/**
			 * Fired when the selection of the text field changes, either by the user or through the "setSelection*"
			 * functions.
			 */
			SelectionChanged = 1 << 2,

			/**
			 * Fired when the user selects text or moves the cursor around inside the text field.
			 */
			UserSelectionChanged = 1 << 3,

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
	 * Default constructor: creates a text with an empty content.
	 */
	TextField();

	/**
	 * Constructor: creates a text with the specified initial contents.
	 */
	TextField(std::string text);

	/**
	 * Destructor.
	 */
	virtual ~TextField();

	/**
	 * Changes the text field's contents.
	 */
	void setText(std::string text);

	/**
	 * Returns the text field's contents.
	 */
	const std::string & getText() const;

	/**
	 * Changes the cursor index, setting both the starting and ending index of the selection to the specified
	 * position.
	 */
	void setCursorPosition(std::size_t position);

	/**
	 * Changes the starting and ending index of the selection.
	 */
	void setSelection(std::size_t start, std::size_t end);

	/**
	 * Changes the starting index of the selection.
	 */
	void setSelectionStart(std::size_t start);

	/**
	 * Changes the ending index of the selection. This affects the current cursor position.
	 */
	void setSelectionEnd(std::size_t end);

	/**
	 * Returns the starting index of the selection.
	 */
	std::size_t getSelectionStart() const;

	/**
	 * Returns the ending index of the selection. This is the current cursor position.
	 */
	std::size_t getSelectionEnd() const;

	/**
	 * Returns the left index of the selection.
	 */
	std::size_t getSelectionLeft() const;

	/**
	 * Returns the right index of the selection.
	 */
	std::size_t getSelectionRight() const;

	/**
	 * Returns the length of the selection.
	 */
	std::size_t getSelectionLength() const;

	/**
	 * Returns true if the current cursor position is on the left end of the selection, and false if it is on the
	 * right end of the selection.
	 */
	bool isSelectionLeft() const;

	/**
	 * Returns true if any text is selected, and false if no text is selected.
	 */
	bool isTextSelected() const;

	/**
	 * Registers a function to be called whenever the text field is modified.
	 */
	CallbackHandle<Event> addEventCallback(EventFunc<Event> func, int typeFilter, int order = 0);

	/**
	 * Calls all registered functions without changing the text field's current state.
	 */
	void fireEvent(Event event);

private:

	void onRepaint(Canvas & canvas) override;

	void updateConfig();

	void handleKeyEvent(KeyEvent event);

	void replaceSelectedText(std::string replacement);

	void handleRelativeMove(int offset);
	void handleAbsoluteMove(int index);

	void handleDelete();
	void handleBackspace();

	pieces::Text * myText;
	CallbackManager<Event> myEventCallbacks;
	Range<std::size_t> mySelection;
	bool myIsSelectModifierPressed;
	bool myIsWordModifierPressed;
	MouseMonitor myMouseMonitor;
};

}

#endif
