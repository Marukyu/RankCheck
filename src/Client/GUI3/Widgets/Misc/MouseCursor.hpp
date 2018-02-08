#ifndef SRC_CLIENT_GUI3_WIDGETS_MISC_MOUSECURSOR_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_MISC_MOUSECURSOR_HPP_

#include <Client/GUI3/Widget.hpp>

namespace gui3
{

/**
 * Renders a graphical mouse cursor to the container it is placed in.
 * 
 * Uses the mouse cursor override of the innermost hovered widget that specifies it.
 */
class MouseCursor : public Widget
{
public:

	/**
	 * Default constructor.
	 */
	MouseCursor();

	/**
	 * Destructor.
	 */
	virtual ~MouseCursor();

private:

	void determineCursorImage();

	void setImage(Ptr<res::Image> image);
	Ptr<res::Image> getImage() const;

	void handleStateEvent(StateEvent event);
	void handleParentMouseMove(MouseEvent event);
	void updateImage();

	sf::FloatRect getCursorRect() const;

	virtual void onRepaint(Canvas & canvas) override;
	virtual void onRender(sf::RenderTarget& target, sf::RenderStates states) const;

	sf::Vector2f myMousePosition;
	Ptr<res::Image> myImage;

	CallbackHandle<MouseEvent> myParentMouseCallback;
};

}

#endif
