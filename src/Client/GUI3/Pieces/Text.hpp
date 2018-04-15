#ifndef SRC_CLIENT_GUI3_PIECES_TEXT_HPP_
#define SRC_CLIENT_GUI3_PIECES_TEXT_HPP_

#include <Client/Graphics/BitmapText.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Piece.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <string>

namespace sf
{
class RenderTarget;
}

namespace gui3
{
class Canvas;
class Widget;

namespace pieces
{

class Text : public Piece
{
public:

	static constexpr float ALIGN_TOP = 0.f;
	static constexpr float ALIGN_LEFT = 0.f;
	static constexpr float ALIGN_CENTER = .5f;
	static constexpr float ALIGN_BOTTOM = 1.f;
	static constexpr float ALIGN_RIGHT = 1.f;

	Text();
	virtual ~Text();

	/**
	 * Draws the text to the specified canvas.
	 */
	virtual void paint(Canvas & canvas) override;

	/**
	 * Renders the text to the specified target.
	 */
	virtual void render(sf::RenderTarget & target, sf::RenderStates states) override;

	/**
	 * Sets/gets the displayed text string.
	 */
	void setString(std::string text);
	const std::string & getString() const;

	/**
	 * Sets/gets the text alignment as a relative anchor value.
	 * 
	 * 0.0 = left/top, 0.5 = center, 1.0 = right/bottom. In-between values are possible.
	 */
	void setAlignmentX(float xAlign);
	void setAlignmentY(float yAlign);
	void setAlignment(float xAlign, float yAlign);
	void setAlignment(sf::Vector2f align);
	sf::Vector2f getAlignment() const;

	/**
	 * Sets/gets the text's base color.
	 */
	void setColor(sf::Color color);
	sf::Color getColor() const;

	/**
	 * Sets/gets the text's scaling factor.
	 */
	void setScale(sf::Vector2f scale);
	sf::Vector2f getScale() const;

	/**
	 * Defines possible text behaviors for when the size limit is reached.
	 * 
	 * Ignore = Allow text to go past the bounding box.
	 * Downscale = Shrink text to fit the bounding box.
	 * Clip = Render only the part of the text that is within the bounding box.
	 */
	enum MaxSizeBehavior
	{
		Downscale,
		Ignore,
		Clip
	};

	/**
	 * Sets/gets the text's behavior when reaching the size limit.
	 */
	void setMaxSizeBehavior(MaxSizeBehavior behavior);
	MaxSizeBehavior getMaxSizeBehavior() const;

	/**
	 * Sets/gets the offset by which the text should be shifted visually. This is relevant when using the Clip max size
	 * behavior to change the displayed text region.
	 */
	void setClipOffset(sf::Vector2f offset);
	sf::Vector2f getClipOffset() const;

	/**
	 * Enables/disables coordinate rounding for the text position to prevent blurriness in an unscaled GUI.
	 */
	void setPositionRounded(bool rounded);
	bool isPositionRounded() const;

	/**
	 * Sets/gets the name of the font that this text should use. Set to an empty string to use the default font.
	 * 
	 * By default, the appplication configuration's "gui.components.text.font" is used.
	 */
	void setFont(std::string fontName);
	std::string getFont() const;
	bool isFontDefault() const;

	/**
	 * Returns the text object's bounding box (or its size).
	 */
	sf::FloatRect getTextRect() const;
	sf::Vector2f getTextPosition() const;
	sf::Vector2f getTextSize() const;

	/**
	 * Returns a rectangle surrounding the characters in the specified range.
	 */
	sf::FloatRect getSelectionBox(std::size_t start, std::size_t length) const;

	/**
	 * Returns the index of the character at the specified position, relative to the text object.
	 * 
	 * Returns std::string::npos if no character is at the specified position.
	 */
	std::size_t getCharacterAtPosition(sf::Vector2f position) const;

protected:

	void onInit() override;
	void onMove() override;
	void onResize() override;

private:

	/**
	 * Called whenever a state event is fired on the managed widget.
	 */
	void handleStateEvent(StateEvent event);

	/**
	 * Updates the text object's position.
	 */
	void updateTextPosition();

	/**
	 * Assigns the font to the text object without modifying the "default font" flag.
	 */
	void loadFont(std::string fontName);

	/**
	 * Reloads/reassigns the font to the text object.
	 */
	void reloadFont();

	/**
	 * Called whenever the configuration changes.
	 */
	void updateConfig();

	/**
	 * Called whenever the color needs to be updated.
	 */
	void updateColor();

	BitmapText myText;

	Ptr<res::Font> myFont;
	std::string myFontName;

	sf::FloatRect myBoundingBox;
	sf::Vector2f myAlignment;
	MaxSizeBehavior myMaxSizeBehavior;
	sf::Color myColor;
	bool myIsPositionRounded;

};

}

}

#endif
