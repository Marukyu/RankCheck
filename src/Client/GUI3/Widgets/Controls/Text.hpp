#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_TEXT_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_TEXT_HPP_

#include <Client/GUI3/Widget.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

namespace gui3
{

namespace pieces
{
class Text;
}

class Canvas;

/**
 * Text label.
 */
class Text : public Widget
{
public:

	static constexpr float ALIGN_LEFT = 0;
	static constexpr float ALIGN_TOP = 0;
	static constexpr float ALIGN_CENTER = 0.5;
	static constexpr float ALIGN_RIGHT = 1;
	static constexpr float ALIGN_BOTTOM = 1;

	/**
	 * Default constructor: creates a text with an empty label.
	 */
	Text();

	/**
	 * Constructor: creates a text with the specified label.
	 */
	Text(std::string text);

	/**
	 * Destructor.
	 */
	virtual ~Text();

	/**
	 * Changes the text's label.
	 */
	void setText(std::string text);

	/**
	 * Returns the text's label.
	 */
	const std::string & getText() const;

	/**
	 * Changes the text's alignment.
	 */
	void setTextAlignment(sf::Vector2f align);
	void setTextAlignment(float xAlign, float yAlign);

	/**
	 * Returns the text's alignment.
	 */
	sf::Vector2f getTextAlignment() const;

	void setTextColor(sf::Color color);

private:

	void onRepaint(Canvas & canvas) override;

	pieces::Text * myText;
};

}

#endif
