#include <Client/Graphics/UtilitiesSf.hpp>
#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Pieces/Text.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <Client/GUI3/Rendering/Primitives/TintedBox.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widget.hpp>
#include <Client/GUI3/Widgets/Misc/Window.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <Shared/Utils/VectorMul.hpp>
#include <algorithm>
#include <fstream>
#include <limits>

namespace gui3
{

Window::Window() :
	myTitleBar(this),
	myBackground(this),
	myResizeOverlay(this),
	myAspectRatio(1.f),
	myIsProportional(false),
	myIsMaximized(false),
	myMinimumSize(0.f, 0.f),
	myMaximumSize(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
	myIsTitleBarDown(false)
{
	myTitleBar.setFocusable(false);
	myCloseButton.setFocusable(false);
	myMaximizeButton.setFocusable(false);
	myBackground.setFocusable(false);

	addWidget(myTitleBar);
	addWidget(myBackground);
	addWidget(myCloseButton);
	addWidget(myMaximizeButton);
	addWidget(myContent);
	addWidget(myResizeOverlay);

	myBackground.setZPosition(-3);
	myContent.setZPosition(-2);
	myTitleBar.setZPosition(-1);
	myResizeOverlay.setZPosition(1);

	myCloseButton.setText("x");
	myMaximizeButton.setText("+");

	myCloseButton.addMouseCallback([this](MouseEvent event)
	{
		myCloseButton.sendToFront();
	}, MouseEvent::Enter);

	myCloseButton.addActionCallback([this]()
	{
		setVisible(false);
	});

	myMaximizeButton.addMouseCallback([this](MouseEvent event)
	{
		myMaximizeButton.sendToFront();
	}, MouseEvent::Enter);

	// Button position update must happen early so that decoration widgets get the correct size values.
	addStateCallback([this](StateEvent event)
	{
		updateContentSize();
	}, StateEvent::Resized, -1);

	addStateCallback([this](StateEvent event)
	{
		handleStateEvent(event);
	}, StateEvent::Any);

	setRect(0, 0, 100, 100);
}

Window::~Window()
{
}

void Window::add(Ptr<Widget> widget)
{
	myContent.add(widget);
}

void Window::remove(Widget& widget)
{
	myContent.remove(widget);
}

void Window::setMaximized(bool maximized)
{
	if (!myIsMaximized && maximized)
	{
		myIsMaximized = true;
		myUnmaximizedRect = getRect();
		updateMaximizedSize();
	}
	else if (myIsMaximized && !maximized)
	{
		myIsMaximized = false;
		setRect(myUnmaximizedRect);
	}
}

bool Window::isMaximized() const
{
	return myIsMaximized;
}

void Window::setTitle(std::string title)
{
	myTitleBar.setTitle(title);
}

std::string Window::getTitle() const
{
	return myTitleBar.getTitle();
}

void Window::setResizable(bool resizable)
{
	myResizeOverlay.setVisible(resizable);
}

bool Window::isResizable() const
{
	return myResizeOverlay.isVisible();
}

void Window::setMovable(bool movable)
{
	myTitleBar.setEnabled(movable);
}

bool Window::isMovable() const
{
	return myTitleBar.isEnabled();
}

void Window::setMaximizable(bool maximizable)
{
	myMaximizeButton.setEnabled(maximizable);
}

bool Window::isMaximizable() const
{
	return myMaximizeButton.isEnabled();
}

void Window::setClosable(bool closable)
{
	myCloseButton.setEnabled(closable);
}

bool Window::isClosable() const
{
	return myCloseButton.isEnabled();
}

void Window::setBorderless(bool borderless)
{
	if (isBorderless() != borderless)
	{
		sf::Vector2f contentSize = getContentSize();

		myTitleBar.setVisible(!borderless);
		myCloseButton.setVisible(!borderless);
		myMaximizeButton.setVisible(!borderless);

		updateTitleBarPlacement();

		if (isMaximized())
		{
			updateContentSize();
		}
		else
		{
			setContentSize(contentSize);
		}
	}
}

bool Window::isBorderless() const
{
	return !myTitleBar.isVisible();
}

void Window::setProportional(bool proportional)
{
	if (myIsProportional != proportional)
	{
		myIsProportional = proportional;
	}
}

bool Window::isProportional() const
{
	return myIsProportional;
}

void Window::setAspectRatio(float ratio)
{
	if (myAspectRatio != ratio)
	{
		myAspectRatio = ratio;
		if (isProportional())
		{
			setContentSize(applyProportionsToContentSize(getContentSize(), ProportionMode::AxisX));
		}
	}
}

float Window::getAspectRatio() const
{
	return myAspectRatio;
}

void Window::setMaximumSize(sf::Vector2f maxSize)
{
	myMaximumSize = maxSize;
}

sf::Vector2f Window::getMaximumSize() const
{
	return myMaximumSize;
}

void Window::setMinimumSize(sf::Vector2f minSize)
{
	myMinimumSize = minSize;
}

sf::Vector2f Window::getMinimumSize() const
{
	return sf::Vector2f(std::max(std::max(myMinimumSize.x, getTitleBarHeight() * 2.f), 2.f * getResizeMargin()),
		std::max(myMinimumSize.y, getTitleBarHeight() + 2.f * getResizeMargin()));
}

Window::Titlebar::Titlebar(Window * window) :
	myWindow(window),
	myIsDragging(false)
{
	myText = addPiece<pieces::Text>();

	window->addStateCallback([this](StateEvent event)
	{
		handleWindowStateEvent(event);
	}, StateEvent::Any);

	addStateCallback([this](StateEvent event)
	{
		repaint();
	}, StateEvent::UsabilityChanged);

	addMouseCallback([this](MouseEvent event)
	{
		handleMouseEvent(event);
	}, MouseEvent::Any);
}

Window::Titlebar::~Titlebar()
{
}

void Window::setContentSize(float contentWidth, float contentHeight)
{
	setContentSize(sf::Vector2f(contentWidth, contentHeight));
}

void Window::setContentSize(sf::Vector2f contentSize)
{
	setSize(convertContentSizeToFrameSize(contentSize));
}

sf::Vector2f Window::getContentSize() const
{
	return myContent.getSize();
}

void Window::Titlebar::setTitle(std::string title)
{
	myText->setString(title);
}

std::string Window::Titlebar::getTitle() const
{
	return myText->getString();
}

void Window::Titlebar::onRepaint(Canvas& canvas)
{
	static cfg::Color tbColor("gui.widgets.window.titleBar.color");

	int tbFlags = primitives::Box::Normal;

	if (myWindow->isFocused())
	{
		tbFlags |= primitives::Box::Focused;
	}

	if (myIsDragging)
	{
		tbFlags |= primitives::Box::Pressed;
	}

	if (!isEnabled())
	{
		tbFlags |= primitives::Box::Disabled;
	}

	canvas.draw(primitives::TintedBox(primitives::Box(getBaseRect(), tbFlags), config().get(tbColor)));
	myText->paint(canvas);
}

void Window::Titlebar::handleWindowStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::FocusGained:
	case StateEvent::FocusLost:
		repaint();
		break;

	case StateEvent::ParentChanged:
		updateWindowParent();
		break;

	default:
		break;
	}
}

void Window::Titlebar::handleMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ButtonDown:
		if (event.button == MouseEvent::Left && isEnabled() && !myWindow->isMaximized())
		{
			myIsDragging = true;
			repaint();
		}
		else if (event.button == MouseEvent::Middle)
		{
			myWindow->sendToBack();
		}
		break;

	case MouseEvent::ButtonUp:
		if (event.button == MouseEvent::Left)
		{
			myIsDragging = false;
			repaint();
		}
		break;

	default:
		break;
	}
}

void Window::Titlebar::handleParentMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ButtonDown:
		if (event.button == MouseEvent::Left)
		{
			myDragOffset = event.position - myWindow->getPosition();
		}
		break;

	case MouseEvent::Move:
		if (myIsDragging)
		{
			myWindow->setPosition(event.position - myDragOffset);
		}
		break;

	default:
		break;
	}
}

void Window::Titlebar::updateWindowParent()
{
	if (myWindow->getParent() == nullptr)
	{
		// Remove callback.
		myParentMouseCallback = Callback<MouseEvent>();
	}
	else
	{
		// Add callback to parent.
		myParentMouseCallback = myWindow->getParent()->addMouseCallback([this](MouseEvent event)
		{
			handleParentMouseEvent(event);
		}, MouseEvent::Any);
	}
}

Window::Background::Background(Window * window) :
	myWindow(window)
{
}

Window::Background::~Background()
{
}

void Window::Background::onRepaint(Canvas& canvas)
{
	canvas.draw(primitives::Box(getBaseRect(), primitives::Box::Background));
}

Window::ResizeOverlay::ResizeOverlay(Window* window) :
	myWindow(window),
	myIsResizing(false),
	myRecursionLock(false),
	myResizeModeX(ResizeMode::None),
	myResizeModeY(ResizeMode::None)
{
	addStateCallback([=](StateEvent event)
	{
		handleStateEvent(event);
	}, StateEvent::Any);
	addMouseCallback([=](MouseEvent event)
	{
		handleMouseEvent(event);
	}, MouseEvent::Any);
}

Window::ResizeOverlay::~ResizeOverlay()
{
}

bool Window::ResizeOverlay::isResizing() const
{
	return myIsResizing;
}

bool Window::ResizeOverlay::testMouseOver(sf::Vector2f pos) const
{
	if (myWindow->isMaximized())
	{
		return false;
	}

	float margin = -getResizeMargin();
	return getBaseRect().contains(pos) && !expandRect(getBaseRect(), sf::Vector2f(margin, margin)).contains(pos);
}

void Window::ResizeOverlay::updateResizeMode(sf::Vector2f mousePos)
{
	if (mousePos.x <= getResizeMargin())
	{
		myResizeModeX = ResizeMode::Negative;
	}
	else if (mousePos.x >= getSize().x - getResizeMargin())
	{
		myResizeModeX = ResizeMode::Positive;
	}
	else
	{
		myResizeModeX = ResizeMode::None;
	}

	if (mousePos.y <= getResizeMargin())
	{
		myResizeModeY = ResizeMode::Negative;
	}
	else if (mousePos.y >= getSize().y - getResizeMargin())
	{
		myResizeModeY = ResizeMode::Positive;
	}
	else
	{
		myResizeModeY = ResizeMode::None;
	}

	if (myResizeModeX != ResizeMode::None || myResizeModeY != ResizeMode::None)
	{
		setMouseCursorOverride(myResizeCursor);
	}
}

float Window::ResizeOverlay::getResizeMargin() const
{
	return myWindow->getResizeMargin();
}

sf::Vector2f Window::ResizeOverlay::getScaleFactor() const
{
	return sf::Vector2f((float) myResizeModeX, (float) myResizeModeY);
}

sf::Vector2f Window::ResizeOverlay::getShiftFactor() const
{
	return sf::Vector2f(std::min((float) myResizeModeX, 0.f), std::min((float) myResizeModeY, 0.f));
}

void Window::ResizeOverlay::handleMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ButtonDown:
		if (event.button == MouseEvent::Left)
		{
			updateResizeMode(event.position);
			myDragOrigin = event.position;
			myInitialWindowPosition = myWindow->getPosition();
			myInitialWindowSize = myWindow->getSize();
			myIsResizing = true;
		}
		break;

	case MouseEvent::Move:
		if (!isResizing())
		{
			updateResizeMode(event.position);
		}
		else if (!myRecursionLock)
		{
			sf::Vector2f eventPos = event.position - myDragOrigin;
			sf::Vector2f windowPositionDiff = transformVector(myWindow->getInverseTransform(),
				myInitialWindowPosition - myWindow->getPosition());

			sf::Vector2f dragPosition = eventPos - windowPositionDiff;

			sf::Vector2f scaleFactor = getScaleFactor();
			sf::Vector2f shiftFactor = getShiftFactor();
			sf::Vector2f newWindowSize = myInitialWindowSize + scaleFactor * dragPosition;

			ProportionMode proportionMode = ProportionMode::Diagonal;

			if (myResizeModeX == ResizeMode::None)
			{
				proportionMode = ProportionMode::AxisY;
			}
			else if (myResizeModeY == ResizeMode::None)
			{
				proportionMode = ProportionMode::AxisX;
			}

			newWindowSize = myWindow->convertFrameSizeToContentSize(newWindowSize);
			newWindowSize = myWindow->applyProportionsToContentSize(newWindowSize, proportionMode);
			newWindowSize = myWindow->applySizeRestrictionsToContentSize(newWindowSize);
			newWindowSize = myWindow->convertContentSizeToFrameSize(newWindowSize);

			sf::Vector2f windowSizeChange = newWindowSize - myInitialWindowSize;
			sf::Vector2f windowPositionChange = transformVector(myWindow->getTransform(),
				windowSizeChange * shiftFactor);
			sf::Vector2f newWindowPosition = myInitialWindowPosition + windowPositionChange;

			myRecursionLock = true;
			myWindow->setPosition(newWindowPosition);
			myWindow->setSize(newWindowSize);
			myRecursionLock = false;
		}
		break;

	case MouseEvent::Leave:
		if (!isResizing())
		{
			myResizeModeX = ResizeMode::None;
			myResizeModeY = ResizeMode::None;
		}
		break;

	case MouseEvent::ButtonUp:
		if (event.button == MouseEvent::Left)
		{
			myIsResizing = false;
			myWindow->updateTitleBarPlacement();
			myWindow->restrictWindowPosition();
		}
		break;

	default:
		break;
	}
}

void Window::ResizeOverlay::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::ResourcesChanged:
		myResizeCursor = resources().acquireImage("gfx/cursor-resize.png");
		break;

	default:
		break;
	}
}

void Window::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::ConfigChanged:
		setContentSize(getContentSize());
		updateButtonSizes();
		repaint();
		break;

	case StateEvent::Moved:
		updateTitleBarPlacement();
		restrictWindowPosition();
		break;

	case StateEvent::ParentBoundsChanged:
		updateMaximizedSize();
		restrictWindowPosition();
		break;

	default:
		break;
	}
}

void Window::updateContentSize()
{
	float margin;
	float tbHeight;
	float tbYPos;
	float contentYPos;

	if (isBorderless())
	{
		margin = 0;
		tbHeight = 0;
		tbYPos = 0;
		contentYPos = 0;
	}
	else
	{
		margin = getMargin();
		tbHeight = getTitleBarHeight() - getOverlap();

		if (isTitleBarDown())
		{
			tbYPos = getSize().y - getTitleBarHeight();
			contentYPos = 0;
		}
		else
		{
			tbYPos = 0;
			contentYPos = tbHeight;
		}
	}

	sf::Vector2f contentSize = convertFrameSizeToContentSize(getSize());

	myCloseButton.setPosition(getSize().x - myCloseButton.getSize().x, tbYPos);
	myMaximizeButton.setPosition(myCloseButton.getPosition().x - myCloseButton.getSize().x + getOverlap(), tbYPos);

	myBackground.setRect(0, contentYPos, getSize().x, getSize().y - tbHeight);
	myContent.setRect(margin, contentYPos + margin, contentSize.x, contentSize.y);
	myTitleBar.setRect(0, tbYPos, getSize().x - getTitleBarRightOffset(), getTitleBarHeight());
	myResizeOverlay.setRect(0, 0, getSize().x, getSize().y);
	//myResizeOverlay.setRect(0, contentYPos, getSize().x, getSize().y - tbHeight);
}

void Window::updateButtonSizes()
{
	sf::Vector2f buttonSize(getTitleBarHeight(), getTitleBarHeight());

	myCloseButton.setSize(buttonSize);
	myMaximizeButton.setSize(buttonSize);

	fireStateEvent(StateEvent::Resized);
}

void Window::updateTitleBarPlacement()
{
	if (myResizeOverlay.isResizing())
	{
		return;
	}

	setTitleBarDown(getTitleBarDownCondition());
}

void Window::updateMaximizedSize()
{
	if (isMaximized() && getParent() != nullptr)
	{
		setRect(0, 0, getParent()->getSize().x, getParent()->getSize().y);
	}
}

void Window::restrictWindowPosition()
{
	if (getParent() == nullptr || myResizeOverlay.isResizing() || isMaximized())
	{
		return;
	}

	sf::Vector2f position = getPosition();
	sf::Vector2f size = getSize();
	sf::Vector2f parentSize = getParent()->getSize();
	sf::Vector2f scale(vectorLength(getTransform().transformPoint(1, 0) - getTransform().transformPoint(0, 0)),
		vectorLength(getTransform().transformPoint(0, 1) - getTransform().transformPoint(0, 0)));

	position.x = std::max(position.x, (getVisibilityThreshold() - size.x + getTitleBarRightOffset()) * scale.x);
	position.x = std::min(position.x, parentSize.x - getVisibilityThreshold() * scale.x);

	position.y = std::max(position.y, (getOverlap() - size.y) * scale.y);
	position.y = std::min(position.y, parentSize.y - getTitleBarHeight() * scale.y);

	setPosition(position);
}

void Window::setTitleBarDown(bool down)
{
	if (myIsTitleBarDown != down)
	{
		myIsTitleBarDown = down;

		sf::Transform transform;
		transform.translate(getWindowOrigin());
		setInternalTransform(transform);

		updateContentSize();
	}
}

bool Window::isTitleBarDown() const
{
	return myIsTitleBarDown;
}

bool Window::getTitleBarDownCondition() const
{
	if (isBorderless())
	{
		return false;
	}

	if (getTransform().transformPoint(sf::Vector2f(0, getTitleBarDownThreshold()) - getWindowOrigin()).y <= 0)
	{
		return true;
	}

	return false;
}

sf::Vector2f Window::convertFrameSizeToContentSize(sf::Vector2f frameSize) const
{
	float tbHeight;
	float margin;

	if (isBorderless())
	{
		tbHeight = 0;
		margin = 0;
	}
	else
	{
		tbHeight = getTitleBarHeight() - getOverlap();
		margin = getMargin();
	}

	return sf::Vector2f(frameSize.x - margin * 2, frameSize.y - tbHeight - margin * 2);
}

sf::Vector2f Window::convertContentSizeToFrameSize(sf::Vector2f contentSize) const
{
	float tbHeight;
	float margin;

	if (isBorderless())
	{
		tbHeight = 0;
		margin = 0;
	}
	else
	{
		tbHeight = getTitleBarHeight() - getOverlap();
		margin = getMargin();
	}

	return sf::Vector2f(contentSize.x + margin * 2, contentSize.y + tbHeight + margin * 2);
}

sf::Vector2f Window::applyProportionsToContentSize(sf::Vector2f contentSize, ProportionMode mode) const
{
	if (isProportional())
	{
		float ratio = getAspectRatio();

		switch (mode)
		{
		case ProportionMode::AxisX:
			return sf::Vector2f(contentSize.x, ratio == 0.f ? contentSize.y : contentSize.x / ratio);
		case ProportionMode::AxisY:
			return sf::Vector2f(contentSize.y * ratio, contentSize.y);
		case ProportionMode::Diagonal:
		{
			sf::Vector2f proportion = normalize(sf::Vector2f(ratio, 1.f));
			float projection = dotProduct(contentSize, proportion);
			return proportion * projection;
		}
		default:
			return contentSize;
		}
	}
	else
	{
		return contentSize;
	}
}

sf::Vector2f Window::applySizeRestrictionsToContentSize(sf::Vector2f contentSize) const
{
	sf::Vector2f minSize = applyProportionsToContentSize(getMinimumSize(), ProportionMode::AxisX);
	sf::Vector2f maxSize = applyProportionsToContentSize(getMaximumSize(), ProportionMode::AxisX);
	contentSize.x = clamp(minSize.x, contentSize.x, maxSize.x);
	contentSize.y = clamp(minSize.y, contentSize.y, maxSize.y);
	return contentSize;
}

sf::Vector2f Window::getWindowOrigin() const
{
	return sf::Vector2f(0, isTitleBarDown() ? getTitleBarHeight() - getOverlap() : 0);
}

float Window::getMargin() const
{
	static cfg::Float margin("gui.widgets.window.margin");
	return config().get(margin);
}

float Window::getOverlap() const
{
	static cfg::Float overlap("gui.widgets.window.overlap");
	return config().get(overlap);
}

float Window::getVisibilityThreshold() const
{
	static cfg::Float visibilityDownThreshold("gui.widgets.window.visibilityThreshold");
	return config().get(visibilityDownThreshold);
}

float Window::getTitleBarRightOffset() const
{
	return getSize().x - myMaximizeButton.getPosition().x - getOverlap();
}

float Window::getTitleBarHeight() const
{
	static cfg::Float titleBarHeight("gui.widgets.window.titleBar.height");
	return config().get(titleBarHeight);
}

float Window::getTitleBarDownThreshold() const
{
	return getTitleBarHeight() - getVisibilityThreshold() - getOverlap();
}

float Window::getResizeMargin() const
{
	static cfg::Float resizeMargin("gui.widgets.window.resizeMargin");
	return config().get(resizeMargin);
}

}
