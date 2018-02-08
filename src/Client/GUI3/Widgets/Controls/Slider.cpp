#include <Client/GUI3/Pieces/Button.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widgets/Controls/Slider.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <cmath>

namespace gui3
{
static cfg::Float configHandleSize("gui.widgets.slider.handle.size");
static cfg::Float configScrollSpeed("gui.widgets.slider.scrolling.speed");
static cfg::Float configAnimationSpeed("gui.widgets.slider.scrolling.animationSpeed");

Slider::Slider() :
	myMouseMonitor(*this),
	myValue(0),
	myTargetValue(0),
	myMinimumValue(0),
	myMaximumValue(10),
	myDirection(Direction::Auto),
	myScrollSpeed(1),
	mySmoothScroll(true)
{
	myHandle = addPiece<pieces::Button>();
	myHandle->clearResizeFunction();

	addMouseCallback([this](MouseEvent event)
	{
		handleMouseEvent(event);
	}, MouseEvent::Any);

	addStateCallback([this](StateEvent event)
	{
		handleStateEvent(event);
	}, StateEvent::Any);

	addTickCallback([this]()
	{
		onTick();
	});

	updateHandle();
}

Slider::~Slider()
{
}

void Slider::setMinimumValue(float minimum)
{
	setBounds(minimum, getMaximumValue());
}

void Slider::setMaximumValue(float maximum)
{
	setBounds(getMinimumValue(), maximum);
}

void Slider::setBounds(float minimum, float maximum)
{
	if (myMinimumValue != minimum || myMaximumValue != maximum)
	{
		myMinimumValue = minimum;
		myMaximumValue = maximum;
		updateHandle();
		setValue(getValue());
	}
}

float Slider::getMinimumValue() const
{
	return myMinimumValue;
}

float Slider::getMaximumValue() const
{
	return myMaximumValue;
}

void Slider::setValue(float value)
{
	setValuePreserveMomentum(value);
	setTargetValue(getValue());
}

float Slider::getValue() const
{
	return myValue;
}

void Slider::setDirection(Direction direction)
{
	if (myDirection != direction)
	{
		myDirection = direction;
		updateHandle();
	}
}

Slider::Direction Slider::getDirection() const
{
	return myDirection;
}

Slider::Direction Slider::getCurrentDirection() const
{
	if (myDirection == Auto)
	{
		if (getSize().x > getSize().y)
		{
			return Horizontal;
		}
		else
		{
			return Vertical;
		}
	}
	else
	{
		return myDirection;
	}
}

void Slider::setScrollSpeed(float scrollSpeed)
{
	myScrollSpeed = scrollSpeed;
}

float Slider::getScrollSpeed() const
{
	return myScrollSpeed;
}

void Slider::setSmoothScrollingEnabled(bool smooth)
{
	mySmoothScroll = smooth;
}

bool Slider::isSmoothScrollingEnabled() const
{
	return mySmoothScroll;
}

CallbackHandle<Slider::Event> Slider::addEventCallback(EventFunc<Event> func, int typeFilter, int order)
{
	return myEventCallbacks.addCallback(func, typeFilter, order);
}

void Slider::fireEvent(Event event)
{
	myEventCallbacks.fireCallback(event.type, event);
}

CallbackHandle<MouseEvent> Slider::linkWithWidget(Widget& widget, int order)
{
	return widget.addMouseCallback([this](MouseEvent event)
	{
		handleRemoteMouseEvent(event);
	}, MouseEvent::ScrollX | MouseEvent::ScrollY, order);
}

void Slider::onRepaint(Canvas& canvas)
{
	int flags = primitives::Box::Background | primitives::Box::Dark;

	if (isFocused())
	{
		flags |= primitives::Box::Pressed | primitives::Box::Focused;
	}

	canvas.draw(primitives::Box(getBaseRect(), flags));

	myHandle->paint(canvas);
}

void Slider::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::ConfigChanged:
		updateConfig();
		break;

	case StateEvent::Resized:
		updateHandle();
		break;

	default:
		break;
	}
}

void Slider::handleMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ScrollX:
	case MouseEvent::ScrollY:
		scrollRelative(event.scrollAmount);
		break;

	case MouseEvent::ButtonDown:
		if (event.button == MouseEvent::Left)
		{
			scrollToMousePosition(event.position);
		}
		break;

	case MouseEvent::Move:
		if (myMouseMonitor.isMouseDown(MouseEvent::Left))
		{
			scrollToMousePosition(event.position);
		}
		break;

	default:
		break;
	}
}

void Slider::handleRemoteMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ScrollX:
	case MouseEvent::ScrollY:
		if ((event.type == MouseEvent::ScrollX && getCurrentDirection() == Direction::Horizontal)
			|| (event.type == MouseEvent::ScrollY && getCurrentDirection() == Direction::Vertical))
		{
			scrollRelative(event.scrollAmount);
		}
		break;

	default:
		break;
	}
}

void Slider::scrollRelative(float amount)
{
	amount = -amount;
	amount *= config().get(configScrollSpeed);
	amount *= getScrollSpeed();

	if (isSmoothScrollingEnabled())
	{
		setTargetValue(getTargetValue() + amount);
	}
	else
	{
		setValue(getValue() + amount);
		fireEvent(Event::UserChanged);
	}
}

void Slider::scrollToMousePosition(sf::Vector2f mousePos)
{
	float handleSize = config().get(configHandleSize);
	float coord = (getCurrentDirection() == Horizontal ? mousePos.x : mousePos.y) - handleSize / 2;
	float size = (getCurrentDirection() == Horizontal ? getSize().x : getSize().y) - handleSize;
	float normalizedPosition = coord / size;

	setValue(normalizedPosition * (getMaximumValue() - getMinimumValue()) + getMinimumValue());
	fireEvent(Event::UserChanged);
}

void Slider::setValuePreserveMomentum(float value)
{
	value = clampValue(value);

	if (myValue != value)
	{
		myValue = value;
		updateHandle();
		fireEvent(Event::Changed);
	}
}

void Slider::setTargetValue(float value)
{
	myTargetValue = value;
}

float Slider::getTargetValue() const
{
	return myTargetValue;
}

float Slider::clampValue(float value) const
{
	return clamp<float>(getMinimumValue(), value, getMaximumValue());
}

void Slider::updateConfig()
{
	updateHandle();
}

void Slider::updateHandle()
{
	float handleSize = config().get(configHandleSize);
	float normalizedPosition = (getValue() - getMinimumValue()) / (getMaximumValue() - getMinimumValue());

	if (getCurrentDirection() == Direction::Horizontal)
	{
		myHandle->setRect(normalizedPosition * (getSize().x - handleSize), 0, handleSize, getSize().y);
	}
	else
	{
		myHandle->setRect(0, normalizedPosition * (getSize().y - handleSize), getSize().x, handleSize);
	}
}

void Slider::onTick()
{
	float delta = myScrollTimer.restart().asSeconds();

	if (std::abs(getTargetValue() - getValue()) > 0.00001f)
	{
		float factor = std::pow(0.5, delta * config().get(configAnimationSpeed));
		float value = getValue() * factor + getTargetValue() * (1 - factor);

		// Check is value underflows or overflows slider bounds.
		if (clampValue(value) != value)
		{
			// Stop smooth scrolling immediately at this point.
			setValue(value);
		}
		else
		{
			// Continue smooth scrolling.
			setValuePreserveMomentum(value);
		}

		fireEvent(Event::UserChanged);
	}
	else if (getValue() != getTargetValue())
	{
		setValue(getTargetValue());
		fireEvent(Event::UserChanged);
	}
}

}
