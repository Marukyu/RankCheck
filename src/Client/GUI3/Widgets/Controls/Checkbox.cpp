#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Pieces/Button.hpp>
#include <Client/GUI3/Pieces/Text.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <Client/GUI3/Widgets/Controls/Checkbox.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>

namespace gui3
{

Checkbox::Checkbox() :
	myIsChecked(false)
{
	myButton = addPiece<pieces::Button>();
	myText = addPiece<pieces::Text>();

	myText->setAlignmentX(pieces::Text::ALIGN_LEFT);

	addMouseCallback([this](MouseEvent event)
	{
		if (event.button == MouseEvent::Left)
		{
			toggle();
			fireEvent(Event::UserChanged);
		}
	}, MouseEvent::Click);

	addStateCallback([this](StateEvent event)
	{
		updateConfigValues();
	}, StateEvent::ConfigChanged);

	updateButtonFlags();
}

Checkbox::Checkbox(std::string text, bool checkboxValue) :
	Checkbox()
{
	setText(text);
	setChecked(checkboxValue);
}

Checkbox::~Checkbox()
{

}

void Checkbox::setText(std::string text)
{
	myText->setString(text);
}

const std::string & Checkbox::getText() const
{
	return myText->getString();
}

void Checkbox::toggle()
{
	setChecked(!isChecked());
}

void Checkbox::setChecked(bool checked)
{
	if (myIsChecked != checked)
	{
		myIsChecked = checked;
		updateButtonFlags();
		fireEvent(Event::Changed);
	}
}

bool Checkbox::isChecked() const
{
	return myIsChecked;
}

CallbackHandle<Checkbox::Event> Checkbox::addEventCallback(EventFunc<Event> func, int typeFilter, int order)
{
	return myEventCallbacks.addCallback(func, typeFilter, order);
}

void Checkbox::fireEvent(Event event)
{
	myEventCallbacks.fireCallback(event.type, event);
}

void Checkbox::onRepaint(Canvas & canvas)
{
	myButton->paint(canvas);
	myText->paint(canvas);
}

void Checkbox::updateButtonFlags()
{
	if (myIsChecked)
	{
		myButton->setFlagsOn(primitives::Box::Marked);
		myButton->resetFlags(primitives::Box::Dark);
	}
	else
	{
		myButton->setFlagsOn(primitives::Box::Dark);
		myButton->resetFlags(primitives::Box::Marked);
	}
}

void Checkbox::updateConfigValues()
{
	static cfg::Vector2f configButtonSize("gui.widgets.checkbox.buttonSize");
	sf::Vector2f buttonSize = config().get(configButtonSize);

	static cfg::Float configButtonGap("gui.widgets.checkbox.buttonGap");
	float buttonGap = config().get(configButtonGap);

	myText->setResizeFunction(Piece::generateResizeFunction(buttonSize.x + buttonGap, 0, 0, 0));
}

}
