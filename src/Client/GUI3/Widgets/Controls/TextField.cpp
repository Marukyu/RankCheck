#include <Client/GUI3/Events/Key.hpp>
#include <Client/GUI3/Events/KeyEvent.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Pieces/Text.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <Client/GUI3/Rendering/Primitives/Gradient.hpp>
#include <Client/GUI3/Rendering/Primitives/Outline.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widgets/Controls/TextField.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <algorithm>

namespace gui3
{

TextField::TextField() :
	myMouseMonitor(*this)
{
	myText = addPiece<pieces::Text>();
	myText->setAlignment(pieces::Text::ALIGN_LEFT, pieces::Text::ALIGN_CENTER);

	addStateCallback([this](StateEvent event)
	{
		repaint();
	}, StateEvent::FocusGained | StateEvent::FocusLost);

	addStateCallback([this](StateEvent event)
	{
		updateConfig();
	}, StateEvent::ConfigChanged);

	addMouseCallback([this](MouseEvent event)
	{
		if (event.button == MouseEvent::Left)
		{
			std::size_t pos = myText->getCharacterAtPosition(event.position);
			if (pos != std::string::npos)
			{
				handleAbsoluteMove(pos);
			}
		}
	}, MouseEvent::ButtonDown);

	addMouseCallback([this](MouseEvent event)
	{
		if (myMouseMonitor.isMouseDown(MouseEvent::Left))
		{
			std::size_t pos = myText->getCharacterAtPosition(event.position);
			if (pos != std::string::npos)
			{
				setSelectionEnd(pos);
			}
		}
	}, MouseEvent::Move);

	addKeyboardCallback([this](KeyEvent event)
	{
		handleKeyEvent(event);
	}, KeyEvent::Any);
}

TextField::TextField(std::string text) :
	TextField()
{
	setText(text);
}

TextField::~TextField()
{
}

void TextField::setText(std::string text)
{
	myText->setString(text);
	setSelection(getSelectionStart(), getSelectionEnd());
	fireEvent(Event::Changed);
}

const std::string & TextField::getText() const
{
	return myText->getString();
}

void TextField::setSelection(std::size_t start, std::size_t end)
{
	start = std::min(start, getText().size());
	end = std::min(end, getText().size());

	if (mySelection.start != start || mySelection.end != end)
	{
		mySelection.start = start;
		mySelection.end = end;
		fireEvent(Event::SelectionChanged);
		repaint();
	}
}

void TextField::setCursorPosition(std::size_t position)
{
	setSelection(position, position);
}

void TextField::setSelectionStart(std::size_t start)
{
	setSelection(start, getSelectionEnd());
}

void TextField::setSelectionEnd(std::size_t end)
{
	setSelection(getSelectionStart(), end);
}

std::size_t TextField::getSelectionStart() const
{
	return mySelection.start;
}

std::size_t TextField::getSelectionEnd() const
{
	return mySelection.end;
}

std::size_t TextField::getSelectionLeft() const
{
	return isSelectionLeft() ? getSelectionEnd() : getSelectionStart();
}

std::size_t TextField::getSelectionRight() const
{
	return isSelectionLeft() ? getSelectionStart() : getSelectionEnd();
}

std::size_t TextField::getSelectionLength() const
{
	return getSelectionRight() - getSelectionLeft();
}

bool TextField::isSelectionLeft() const
{
	return getSelectionStart() > getSelectionEnd();
}

bool TextField::isTextSelected() const
{
	return getSelectionStart() != getSelectionEnd();
}

CallbackHandle<TextField::Event> TextField::addEventCallback(EventFunc<Event> func, int typeFilter, int order)
{
	return myEventCallbacks.addCallback(func, typeFilter, order);
}

void TextField::fireEvent(Event event)
{
	myEventCallbacks.fireCallback(event.type, event);
}

void TextField::onRepaint(Canvas& canvas)
{
	int flags = primitives::Box::Background | primitives::Box::Dark;
	if (isFocused())
	{
		flags |= primitives::Box::Focused;
	}
	canvas.draw(primitives::Box(getBaseRect(), flags));

	if (isFocused())
	{
		static cfg::Color topColor("gui.widgets.textField.selection.topColor");
		static cfg::Color bottomColor("gui.widgets.textField.selection.bottomColor");
		static cfg::Color outlineColor("gui.widgets.textField.selection.outlineColor");
		static cfg::Float outlineThickness("gui.widgets.textField.selection.outlineThickness");

		sf::FloatRect selectRect = myText->getSelectionBox(getSelectionLeft(), getSelectionLength());

		primitives::Gradient selectionGradient(selectRect, primitives::Gradient::Vertical, config().get(topColor),
			config().get(bottomColor));
		primitives::Outline selectionOutline(selectRect, -config().get(outlineThickness), config().get(outlineColor));

		if (!isTextSelected())
		{
			myText->paint(canvas);
		}

		canvas.draw(selectionGradient);
		canvas.draw(selectionOutline);

		if (isTextSelected())
		{
			myText->paint(canvas);
		}
	}
	else
	{
		myText->paint(canvas);
	}
}

void TextField::updateConfig()
{
	static cfg::Vector2f configMargins("gui.widgets.textField.margins");
	sf::Vector2f margins = config().get(configMargins);

	myText->setResizeFunction(Piece::generateResizeFunction(margins.x, margins.y));
}

void TextField::handleKeyEvent(KeyEvent event)
{
	switch (event.type)
	{
	case KeyEvent::Input:
	{
		// TODO: Handle unicode better.
		char enteredCharacter = 0;

		if (event.character >= 32 && event.character < 256 && event.character != 127)
		{
			enteredCharacter = (unsigned char) event.character;
		}

		if (enteredCharacter != 0)
		{
			replaceSelectedText(std::string(1, enteredCharacter));
			setCursorPosition(getSelectionLeft() + 1);
		}
		break;
	}

	case KeyEvent::Press:

		if (event.key.isShift())
		{
			myIsSelectModifierPressed = true;
		}

		if (event.key.isControl())
		{
			myIsWordModifierPressed = true;
		}

		switch (Key::toSFML(event.key))
		{
		case sf::Keyboard::Left:
			handleRelativeMove(-1);
			break;

		case sf::Keyboard::Right:
			handleRelativeMove(1);
			break;

		case sf::Keyboard::Up:
			break;

		case sf::Keyboard::Down:
			break;

		case sf::Keyboard::Delete:
			handleDelete();
			break;

		case sf::Keyboard::BackSpace:
			handleBackspace();
			break;

		default:
			break;
		}
		break;

	case KeyEvent::Release:

		if (event.key.isShift())
		{
			myIsSelectModifierPressed = false;
		}

		if (event.key.isControl())
		{
			myIsWordModifierPressed = false;
		}
		break;

	default:
		break;
	}
}

void TextField::replaceSelectedText(std::string replacement)
{
	std::string textPieceLeft = getText().substr(0, getSelectionLeft());
	std::string textPieceRight = getText().substr(getSelectionRight());
	setText(textPieceLeft + replacement + textPieceRight);
	fireEvent(Event::UserChanged);

	setCursorPosition(getSelectionLeft());
}

void TextField::handleRelativeMove(int offset)
{
	handleAbsoluteMove(getSelectionEnd() + offset);
}

void TextField::handleAbsoluteMove(int index)
{
	index = clamp<int>(0, index, getText().size());

	if (myIsSelectModifierPressed)
	{
		setSelectionEnd(index);
	}
	else
	{
		setCursorPosition(index);
	}

	fireEvent(Event::UserSelectionChanged);
}

void TextField::handleDelete()
{
	if (isTextSelected())
	{
		replaceSelectedText("");
	}
	else if (getSelectionRight() < getText().size())
	{
		std::string text = getText();
		text.erase(getSelectionLeft(), 1);
		setText(text);
		fireEvent(Event::UserChanged);
	}
}

void TextField::handleBackspace()
{
	if (isTextSelected())
	{
		replaceSelectedText("");
	}
	else if (getSelectionLeft() != 0)
	{
		std::size_t pos = getSelectionLeft() - 1;
		std::string text = getText();
		text.erase(pos, 1);
		setText(text);
		fireEvent(Event::UserChanged);
		setCursorPosition(pos);
	}
}

}
