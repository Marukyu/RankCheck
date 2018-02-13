/*
 * Interface.cpp
 *
 *  Created on: Jun 15, 2015
 *      Author: marukyu
 */

#include <Client/GUI3/Application.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Interface.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <cmath>
#include <limits>
#include <vector>

using namespace gui3;

Interface::~Interface()
{
	closeWindow();
}

Panel & Interface::getRootContainer()
{
	return myRootContainer;
}

const Panel & Interface::getRootContainer() const
{
	return myRootContainer;
}

Application & Interface::getParentApplication() const
{
	return *myParentApplication;
}

void Interface::setTitle(sf::String title)
{
	myWindowTitle = title;
	myWindow.setTitle(title);
}

sf::String Interface::getTitle() const
{
	return myWindowTitle;
}

void Interface::resize(sf::Vector2u size, bool fullscreen)
{
	if (myWindow.isOpen())
	{
		// If window is or will be fullscreen, recreate it.
		if (myIsFullscreen || fullscreen)
		{
			myWindow.close();
			myHasFocus = true;
			myWindowSize = size;
			myIsFullscreen = fullscreen;
			openWindow();
		}
		else
		{
			myWindow.setSize(size);
		}

		myWindowSize = myWindow.getSize();
	}
	else
	{
		// Assign variables for future window creation.
		myWindowSize = size;
		myIsFullscreen = fullscreen;
	}
}

sf::Vector2i Interface::getPosition() const
{
	return myWindowPosition;
}

sf::Vector2u Interface::getSize() const
{
	return myWindowSize;
}

bool Interface::isFullscreen() const
{
	return myIsFullscreen;
}

bool Interface::isFocused() const
{
	return myHasFocus;
}

void Interface::setUseSystemCursor(bool visible)
{
	if (myIsMouseCursorVisible != visible)
	{
		myIsMouseCursorVisible = visible;
		myWindow.setMouseCursorVisible(myIsMouseCursorVisible);
	}
}

bool Interface::isUsingSystemCursor() const
{
	return myIsMouseCursorVisible;
}

Interface::Interface(Application* parentApplication) :
	myIsFullscreen(false),
	myIsMouseCursorVisible(true),
	myHasFocus(true),
	myRootContainer(this),
	myParentApplication(parentApplication),
	myWindowSize(640, 480),
	myWindowTitle("WOS Application")
{
}

sf::RenderTarget & Interface::getRenderTarget()
{
	return myWindow;
}

const sf::RenderTarget & Interface::getRenderTarget() const
{
	return myWindow;
}

bool Interface::isWindowOpen() const
{
	return myWindow.isOpen();
}

void Interface::openWindow()
{
	sf::ContextSettings context;
	//context.antialiasingLevel = 4;

	if (myIsFullscreen)
	{
		myWindow.create(findGoodVideoMode(myWindowSize), myWindowTitle, sf::Style::Fullscreen, context);
	}
	else
	{
		myWindow.create(sf::VideoMode(myWindowSize.x, myWindowSize.y), myWindowTitle, sf::Style::Default, context);
	}

	myWindowPosition = myWindow.getPosition();
	myWindowSize = myWindow.getSize();

	myWindow.setMouseCursorVisible(myIsMouseCursorVisible);
}

void Interface::closeWindow()
{
	if (isWindowOpen())
	{
		getParentApplication().cleanUpWindowResources();
		myWindow.close();
	}
}

void Interface::process()
{
	myWindowPosition = myWindow.getPosition();

	for (sf::Event event; myWindow.pollEvent(event);)
	{
		processEvent(event);
	}

	myRootContainer.setClippingWidgets(false);
	myRootContainer.setSize(sf::Vector2f(getSize()));

	myRootContainer.fireTick();

	if (myRootContainer.isRepaintNeeded())
	{
		myRootContainer.performRepaint();
	}

	myWindow.clear();
	myWindow.setView(sf::View(sf::FloatRect(0, 0, getSize().x, getSize().y)));

	sf::RenderStates states;
	states.texture = getParentApplication().getMainTexture();
	myRootContainer.onRender(myWindow, states);

	onRender();
	myWindow.display();
}

void Interface::processEvent(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::MouseMoved:
		myRootContainer.fireMouseEvent(
			MouseEvent::generatePositionalEvent(MouseEvent::Move, sf::Vector2f(event.mouseMove.x, event.mouseMove.y)));
		break;

	case sf::Event::MouseEntered:
		myRootContainer.fireMouseEvent(
			MouseEvent::generatePositionalEvent(MouseEvent::Enter, sf::Vector2f(sf::Mouse::getPosition(myWindow))));
		break;

	case sf::Event::MouseLeft:
		myRootContainer.fireMouseEvent(
			MouseEvent::generatePositionalEvent(MouseEvent::Leave, sf::Vector2f(sf::Mouse::getPosition(myWindow))));
		break;

	case sf::Event::MouseButtonPressed:
		myRootContainer.fireMouseEvent(
			MouseEvent::generateButtonEvent(MouseEvent::ButtonDown,
				sf::Vector2f(event.mouseButton.x, event.mouseButton.y),
				MouseEvent::getButtonConstantFromSFML(event.mouseButton.button)));
		break;

	case sf::Event::MouseButtonReleased:
		myRootContainer.fireMouseEvent(
			MouseEvent::generateButtonEvent(MouseEvent::ButtonUp,
				sf::Vector2f(event.mouseButton.x, event.mouseButton.y),
				MouseEvent::getButtonConstantFromSFML(event.mouseButton.button)));
		break;

	case sf::Event::MouseWheelScrolled:
	{
		myRootContainer.fireMouseEvent(
			MouseEvent::generateScrollEvent(MouseEvent::getScrollTypeConstantFromSFML(event.mouseWheelScroll.wheel),
				sf::Vector2f(event.mouseWheelScroll.x, event.mouseWheelScroll.y), event.mouseWheelScroll.delta));
		break;
	}

	case sf::Event::KeyPressed:
		myRootContainer.fireKeyboardEvent(KeyEvent(KeyEvent::Press, Key::fromSFML(event.key.code)));
		break;

	case sf::Event::KeyReleased:
		myRootContainer.fireKeyboardEvent(KeyEvent(KeyEvent::Release, Key::fromSFML(event.key.code)));
		break;

	case sf::Event::TextEntered:
		myRootContainer.fireKeyboardEvent(KeyEvent(KeyEvent::Input, event.text.unicode));
		break;

	case sf::Event::Closed:
		closeWindow();
		break;

	case sf::Event::GainedFocus:
		myHasFocus = true;
		break;

	case sf::Event::LostFocus:
		myHasFocus = false;
		break;

	case sf::Event::Resized:
		myWindowSize.x = event.size.width;
		myWindowSize.y = event.size.height;
		break;

	default:
		break;
	}

	// Allow derived interface to process further events.
	onEvent(event);
}

void Interface::onEvent(const sf::Event& event)
{
}

void Interface::onRender()
{
}

Interface::RootContainer::RootContainer(Interface * parentInterface)
{
	myParentInterface = parentInterface;
	setClippingWidgets(false);
}

Interface::RootContainer::~RootContainer()
{
}

Interface* Interface::RootContainer::getParentInterface() const
{
	return myParentInterface;
}

sf::VideoMode Interface::findGoodVideoMode(sf::Vector2u compare)
{
	const std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();

	if (modes.empty())
	{
		return sf::VideoMode::getDesktopMode();
	}

	sf::VideoMode bestMode = sf::VideoMode::getDesktopMode();

	// Mode "quality", difference between attempted and available mode. The lower, the better.
	int bestModeQuality = std::numeric_limits<int>::max();

	for (unsigned int i = 0; i < modes.size(); ++i)
	{
		const int curModeQuality = std::abs((int) modes[i].width - (int) compare.x)
			+ std::abs((int) modes[i].height - (int) compare.y)
			- modes[i].bitsPerPixel;

		if (curModeQuality < bestModeQuality)
		{
			bestMode = modes[i];
			bestModeQuality = curModeQuality;
		}
	}

	return bestMode;
}

void Interface::setIcon(const sf::Image& icon)
{
	myWindow.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}
