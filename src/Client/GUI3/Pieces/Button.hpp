#ifndef SRC_CLIENT_GUI3_PIECES_BUTTON_HPP_
#define SRC_CLIENT_GUI3_PIECES_BUTTON_HPP_

#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Piece.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <SFML/Graphics/Color.hpp>

namespace gui3
{
class MouseEvent;

namespace pieces
{

class Button : public Piece
{
public:
	Button();
	virtual ~Button();

	/**
	 * Draws the button to the specified canvas.
	 */
	virtual void paint(Canvas & canvas) override;

	/**
	 * Allows setting additional force-enabled/force-disabled box flags for the button.
	 */
	void setFlagsOn(primitives::Box::Flags flags);
	void setFlagsOff(primitives::Box::Flags flags);
	void resetFlags(primitives::Box::Flags flags = primitives::Box::AllFlags);

	/**
	 * Allows changing the button's color.
	 */
	void setColor(sf::Color color);
	void resetColor();
	sf::Color getColor() const;
	bool isColorSet() const;

protected:

	virtual void onInit() override;

private:

	void handleMouseEvent(MouseEvent event);
	void handleStateEvent(StateEvent event);

	int myBoxFlags;
	int myOnBoxFlags;
	int myOffBoxFlags;
	sf::Color myColor;
	bool myIsColorSet;
};

}

}

#endif
