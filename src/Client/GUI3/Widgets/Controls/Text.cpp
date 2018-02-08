#include <Client/GUI3/Pieces/Text.hpp>
#include <Client/GUI3/Widgets/Controls/Text.hpp>

namespace gui3
{

Text::Text()
{
	myText = addPiece<pieces::Text>();
	myText->setPositionRounded(true);
}

Text::Text(std::string text) :
	Text()
{
	setText(text);
}

Text::~Text()
{
}

void Text::setText(std::string text)
{
	myText->setString(text);
}

const std::string & Text::getText() const
{
	return myText->getString();
}

void Text::setTextAlignment(sf::Vector2f align)
{
	myText->setAlignment(align);
}

void Text::setTextAlignment(float xAlign, float yAlign)
{
	setTextAlignment(sf::Vector2f(xAlign, yAlign));
}

sf::Vector2f Text::getTextAlignment() const
{
	return myText->getAlignment();
}

void Text::setTextColor(sf::Color color)
{
	myText->setColor(color);
}

void Text::onRepaint(Canvas& canvas)
{
	myText->paint(canvas);
}

}
