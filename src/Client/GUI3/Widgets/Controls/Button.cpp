#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Pieces/Button.hpp>
#include <Client/GUI3/Pieces/Text.hpp>
#include <Client/GUI3/Widgets/Controls/Button.hpp>
#include <algorithm>

namespace gui3
{

Button::Button()
{
	myButton = addPiece<pieces::Button>();
	myText = addPiece<pieces::Text>();
	myText->setPositionRounded(true);

	addMouseCallback([this](MouseEvent event)
	{
		if (event.button == MouseEvent::Left)
		{
			fireAction();
		}
	}, MouseEvent::Click);
}

Button::Button(std::string text) :
	Button()
{
	setText(text);
}

Button::~Button()
{
}

void Button::setText(std::string text)
{
	myText->setString(std::move(text));
}

const std::string & Button::getText() const
{
	return myText->getString();
}

CallbackHandle<> Button::addActionCallback(EventFunc<> func, int order)
{
	return myActionCallbacks.addCallback(func, 1, order);
}

void Button::fireAction()
{
	myActionCallbacks.fireCallback(1);
}

void Button::onRepaint(Canvas& canvas)
{
	myButton->paint(canvas);
	myText->paint(canvas);
}

}
