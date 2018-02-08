#ifndef SRC_CLIENT_GUI3_PIECE_HPP_
#define SRC_CLIENT_GUI3_PIECE_HPP_

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <memory>

namespace cfg
{
class Config;
}

namespace sf
{
class RenderTarget;
}

namespace gui3
{

class Widget;
class VertexBuffer;
class Canvas;

namespace res
{
class ResourceManager;
}

class Piece
{
public:
	Piece();
	virtual ~Piece();

	bool isComplex() const;

	/**
	 * Draws the piece to the specified canvas.
	 */
	virtual void paint(Canvas & canvas);

	/**
	 * Renders the piece to the specified target.
	 */
	virtual void render(sf::RenderTarget & target, sf::RenderStates states);

	/**
	 * Sets/gets the piece's position.
	 */
	void setPosition(sf::Vector2f position);
	void setPosition(float x, float y);
	sf::Vector2f getPosition() const;

	/**
	 * Sets/gets the piece's size.
	 */
	void setSize(sf::Vector2f size);
	void setSize(float width, float height);
	sf::Vector2f getSize() const;

	/**
	 * Sets/gets the piece's bounding box.
	 */
	void setRect(sf::FloatRect rect);
	void setRect(float left, float top, float width, float height);
	sf::FloatRect getRect() const;

	using ResizeFunc = std::function<sf::FloatRect(sf::Vector2f)>;

	/**
	 * Enables/disables automatically resizing the piece's bounding box to the widget's full size.
	 */
	void setAutoResizeFunction();
	void setResizeFunction(ResizeFunc function);
	void clearResizeFunction();
	void updateSize();

	static ResizeFunc generateResizeFunction();
	static ResizeFunc generateResizeFunction(float horizontalMargin, float verticalMargin);
	static ResizeFunc generateResizeFunction(float leftMargin, float topMargin, float rightMargin, float bottomMargin);

protected:

	/**
	 * Called when the piece is added to a widget. Does nothing by default.
	 */
	virtual void onInit();

	/**
	 * Called when the piece is moved. Repaints by default.
	 */
	virtual void onMove();

	/**
	 * Called when the piece is resized. Repaints by default.
	 */
	virtual void onResize();

	/**
	 * Returns the widget this piece is attached to. Returns a null pointer if it is not currently attached.
	 */
	Widget * getWidget() const;

	void setComplex(bool complex);

	void repaint();

	/**
	 * Convenience/shortcut functions.
	 */
	cfg::Config & config() const;
	res::ResourceManager & resources() const;

private:

	/**
	 * Called whenever a state event is fired on the managed widget.
	 */
	void doAutoResize();

	void setWidget(Widget & widget);

	std::unique_ptr<VertexBuffer> myVertexBuffer;
	Widget * myWidget;
	bool myIsComplex;
	bool myHasResizeFunction;
	ResizeFunc myResizeFunction;
	sf::FloatRect myRect;

	friend class Widget;
};

}

#endif
