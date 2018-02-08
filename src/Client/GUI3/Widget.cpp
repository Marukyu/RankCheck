/*
 * Widget.cpp
 *
 *  Created on: Mar 2, 2015
 *      Author: marukyu
 */

#include <Client/Graphics/UtilitiesSf.hpp>
#include <Client/GUI3/Application.hpp>
#include <Client/GUI3/Container.hpp>
#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/KeyEvent.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Interface.hpp>
#include <Client/GUI3/Piece.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Utils/NullResourceManager.hpp>
#include <Client/GUI3/Widget.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <algorithm>
#include <memory>
#include <vector>

namespace gui3
{
class ResourceManager;
}

namespace gui3
{

Widget::Widget() :
	myIsTransformInvalid(true),
	myIsInverseTransformInvalid(true),
	myIsVisible(true),
	myIsEnabled(true),
	myIsFocusable(true),
	myZPosition(0),
	myRendererOverride(nullptr),
	myParent(nullptr),
	myIsRepaintNeeded(true),
	myIsWeakRepaintNeeded(true),
	myIsComplex(false),
	myHasComplexPiece(false),
	myIsGlobalKeyListener(false)
{
	addStateCallback([this](StateEvent event)
	{
		switch (event.type)
		{
		case StateEvent::RendererChanged:
			repaint();
			break;

		case StateEvent::ParentBoundsChanged:
			repaintWeak();
			break;

		case StateEvent::ParentApplicationChanged:
			submitQueuedInvocations();
			break;

		default:
			break;
		}
	}, StateEvent::RendererChanged | StateEvent::ParentBoundsChanged);
}

Widget::~Widget()
{
	disableAllInvokeLaterFunctions();

	myKeyCallbacks.clearCallbacks();
	myMouseCallbacks.clearCallbacks();
	myStateCallbacks.clearCallbacks();
	myTickCallbacks.clearCallbacks();

	if (myParent)
	{
		myParent->removeWidget(*this);
	}
}

void Widget::setPosition(sf::Vector2f pos)
{
	if (myPosition != pos)
	{
		myPosition = pos;

		invalidateTransform();
		repaintWeak();
		fireStateEvent(StateEvent(StateEvent::Moved));
	}
}

void Widget::setPosition(float x, float y)
{
	setPosition(sf::Vector2f(x, y));
}

sf::Vector2f Widget::getPosition() const
{
	return myPosition;
}

void Widget::setSize(sf::Vector2f size)
{
	if (mySize != size)
	{
		mySize = size;

		repaint();
		fireStateEvent(StateEvent(StateEvent::Resized));
	}
}

void Widget::setSize(float width, float height)
{
	setSize(sf::Vector2f(width, height));
}

sf::Vector2f Widget::getSize() const
{
	return mySize;
}

void Widget::setRect(sf::FloatRect rect)
{
	setRect(rect.left, rect.top, rect.width, rect.height);
}

void Widget::setRect(float x, float y, float width, float height)
{
	setPosition(x, y);
	setSize(width, height);
}

sf::FloatRect Widget::getRect() const
{
	return sf::FloatRect(myPosition.x, myPosition.y, mySize.x, mySize.y);
}

sf::FloatRect Widget::getBaseRect() const
{
	return sf::FloatRect(0, 0, mySize.x, mySize.y);
}

void Widget::setCustomTransform(sf::Transform transform)
{
	if (myCustomTransform == nullptr)
	{
		myCustomTransform = makeUnique<sf::Transform>();
	}

	// This check is OK even if the transform was just created because it is initialized to Identity,
	// which is treated equivalently to the unique pointer being null.
	if (!transformEquals(*myCustomTransform, transform))
	{
		*myCustomTransform = transform;
		invalidateTransform();
		repaintWeak();
		fireStateEvent(StateEvent(StateEvent::CustomTransformChanged));
	}
}

sf::Transform Widget::getCustomTransform() const
{
	return myCustomTransform == nullptr ? sf::Transform::Identity : *myCustomTransform;
}

void Widget::setInternalTransform(sf::Transform transform)
{
	if (myInternalTransform == nullptr)
	{
		myInternalTransform = makeUnique<sf::Transform>();
	}

	// This check is OK even if the transform was just created because it is initialized to Identity,
	// which is treated equivalently to the unique pointer being null.
	if (!transformEquals(*myInternalTransform, transform))
	{
		*myInternalTransform = transform;
		invalidateTransform();
		repaintWeak();
		fireStateEvent(StateEvent(StateEvent::InternalTransformChanged));
	}
}

sf::Transform Widget::getInternalTransform() const
{
	return myInternalTransform == nullptr ? sf::Transform::Identity : *myInternalTransform;
}

bool Widget::testMouseOver(sf::Vector2f pos) const
{
	return getBaseRect().contains(pos);
}

void Widget::setVisible(bool visible)
{
	if (myIsVisible != visible)
	{
		myIsVisible = visible;
		fireStateEvent(StateEvent::VisibilityChanged);

		repaint();
	}
}

bool Widget::isVisible() const
{
	return myIsVisible;
}

void Widget::setEnabled(bool enabled)
{
	if (myIsEnabled != enabled)
	{
		myIsEnabled = enabled;
		fireStateEvent(StateEvent::UsabilityChanged);
	}
}

bool Widget::isEnabled() const
{
	return myIsEnabled;
}

void Widget::acquireFocus()
{
	// If parent exists, tell it to focus this widget. If not, this widget is already focused automatically.
	if (isFocusable() && getParent() != nullptr)
	{
		getParent()->setFocusedWidget(this);
	}
}

void Widget::dropFocus()
{
	if (getParent() != nullptr)
	{
		getParent()->setFocusedWidget(nullptr);
	}
}

void Widget::stealFocus()
{
	// Acquire local focus.
	acquireFocus();

	// Recursively let parents acquire focus.
	if (getParent() != nullptr)
	{
		stealFocus();
	}
}

bool Widget::isFocused() const
{
	// No parent? Return true; root widgets are always focused.
	if (getParent() == nullptr)
	{
		return true;
	}

	// No focus within parent? Return false.
	if (getParent()->getFocusedWidget() != this)
	{
		return false;
	}

	// Recurse to parent.
	return getParent()->isFocused();
}

void Widget::setFocusable(bool focusable)
{
	myIsFocusable = focusable;

	if (!focusable && isFocused() && getParent() != nullptr)
	{
		getParent()->setFocusedWidget(nullptr);
	}
}

bool Widget::isFocusable() const
{
	return myIsFocusable;
}

void Widget::setZPosition(int zpos)
{
	if (myZPosition != zpos)
	{
		if (getParent() != nullptr)
		{
			getParent()->updateWidgetZPosition(*this, zpos);
		}

		myZPosition = zpos;
	}
}

int Widget::getZPosition() const
{
	return myZPosition;
}

void Widget::sendToFront()
{
	if (getParent() != nullptr)
	{
		getParent()->sendWidgetToFront(*this);
	}
}

void Widget::sendToBack()
{
	if (getParent() != nullptr)
	{
		getParent()->sendWidgetToBack(*this);
	}
}

void Widget::setRendererOverride(const Renderer* renderer)
{
	if (myRendererOverride != renderer)
	{
		myRendererOverride = renderer;
		fireStateEvent(StateEvent(StateEvent::RendererChanged));
	}
}

const Renderer* Widget::getRendererOverride() const
{
	return myRendererOverride;
}

const Renderer* Widget::getRenderer() const
{
	// Try to return override first. If no override exists, try to return parent renderer.
	// If no parent exists, return null pointer.
	return myRendererOverride ? myRendererOverride : (getParent() ? getParent()->getRenderer() : nullptr);
}

void Widget::setMouseCursorOverride(Ptr<res::Image> cursor)
{
	myCursorOverride = cursor;
}

Ptr<res::Image> Widget::getMouseCursorOverride() const
{
	return myCursorOverride;
}

CallbackHandle<MouseEvent> Widget::addMouseCallback(EventFunc<MouseEvent> func, int typeFilter, int order)
{
	return myMouseCallbacks.addCallback(func, typeFilter, order);
}

CallbackHandle<KeyEvent> Widget::addKeyboardCallback(EventFunc<KeyEvent> func, int typeFilter, int order)
{
	return myKeyCallbacks.addCallback(func, typeFilter, order);
}

CallbackHandle<StateEvent> Widget::addStateCallback(EventFunc<StateEvent> func, int typeFilter, int order)
{
	return myStateCallbacks.addCallback(func, typeFilter, order);
}

CallbackHandle<> Widget::addTickCallback(EventFunc<> func, int order)
{
	return myTickCallbacks.addCallback(func, 1, order);
}

void Widget::fireMouseEvent(MouseEvent event)
{
	myMouseCallbacks.fireCallback(event.type, event);
}

void Widget::fireKeyboardEvent(KeyEvent event)
{
	myKeyCallbacks.fireCallback(event.type, event);
}

void Widget::fireStateEvent(StateEvent event)
{
	myStateCallbacks.fireCallback(event.type, event);
}

void Widget::fireTick()
{
	myTickCallbacks.fireCallback(1);
}

void Widget::repaint()
{
	repaintImpl(false);
}

void Widget::repaintWeak()
{
	if (!myIsWeakRepaintNeeded && !myIsRepaintNeeded)
	{
		myIsWeakRepaintNeeded = true;

		if (myParent)
		{
			myParent->repaintWidget(*this, false);
		}
	}
}

const sf::Transform & Widget::getTransform() const
{
	if (myIsTransformInvalid)
	{
		myTransform = sf::Transform::Identity;

		myTransform.translate(getPosition());

		if (myCustomTransform != nullptr)
		{
			myTransform.combine(*myCustomTransform);
		}

		if (myInternalTransform != nullptr)
		{
			myTransform.combine(*myInternalTransform);
		}

		myIsTransformInvalid = false;
	}
	return myTransform;
}

const sf::Transform & Widget::getInverseTransform() const
{
	if (myIsInverseTransformInvalid)
	{
		myInverseTransform = getTransform().getInverse();
		myIsInverseTransformInvalid = false;
	}
	return myInverseTransform;
}

bool Widget::isComplex() const
{
	return myIsComplex || myHasComplexPiece;
}

bool Widget::isGlobalKeyListener() const
{
	return myIsGlobalKeyListener;
}

Container * Widget::getParent() const
{
	return myParent;
}

Interface * Widget::getParentInterface() const
{
	if (getParent() == nullptr)
	{
		return nullptr;
	}

	return getParent()->getParentInterface();
}

Application * Widget::getParentApplication() const
{
	Interface * parentInterface = getParentInterface();

	if (parentInterface == nullptr)
	{
		return nullptr;
	}

	return &parentInterface->getParentApplication();
}

void Widget::invokeLater(std::function<void()> function, int order)
{
	clearCompletedInvokeLaterFunctions();

	Invocation invocation;

	invocation.function = function;
	invocation.order = order;

	auto app = getParentApplication();
	if (app != nullptr)
	{
		invocation.queued = false;
		invocation.handle = app->invokeLater(function, order);
	}
	else
	{
		invocation.queued = true;
	}
	myInvocations.push_back(invocation);
}

void Widget::setComplexOverride(bool complex)
{
	if (myIsComplex != complex)
	{
		myIsComplex = complex;
		repaintImpl(true);
	}
}

bool Widget::isComplexOverride() const
{
	return myIsComplex;
}

void Widget::setGlobalKeyListener(bool globalKeyListener)
{
	myIsGlobalKeyListener = globalKeyListener;
}

bool Widget::isRecursiveParent(const Widget& widget) const
{
	if (getParent() == &widget)
	{
		return true;
	}
	else if (getParent() == nullptr)
	{
		return false;
	}
	else
	{
		return getParent()->isRecursiveParent(widget);
	}
}

VertexBuffer & Widget::getVertexBuffer()
{
	return myVertexBuffer;
}

const VertexBuffer& Widget::getVertexBuffer() const
{
	return myVertexBuffer;
}

cfg::Config& Widget::config() const
{
	if (getParentApplication() == nullptr)
	{
		static cfg::Config nullConfig;
		return nullConfig;
	}
	return getParentApplication()->getConfig();
}

res::ResourceManager& Widget::resources() const
{
	if (getParentApplication() == nullptr)
	{
		static res::NullResourceManager nullResourceManager;
		return nullResourceManager;
	}
	return getParentApplication()->getResourceManager();
}

void Widget::setParent(Container * parent)
{
	Application * oldApplication = getParentApplication();
	const Renderer * oldRenderer = getRenderer();

	myParent = parent;
	fireStateEvent(StateEvent::ParentChanged);

	if (getParentApplication() != oldApplication)
	{
		fireStateEvent(StateEvent::ParentApplicationChanged);
		fireStateEvent(StateEvent::ResourcesChanged);
		fireStateEvent(StateEvent::ConfigChanged);
	}

	if (getRenderer() != oldRenderer)
	{
		fireStateEvent(StateEvent::RendererChanged);
	}
}

void Widget::appendPiece(std::unique_ptr<Piece> piece)
{
	myPieces.push_back(std::move(piece));
	myPieces.back()->setWidget(*this);
}

void Widget::invalidateTransform()
{
	myIsTransformInvalid = true;
	myIsInverseTransformInvalid = true;
}

void Widget::onRender(sf::RenderTarget & target, sf::RenderStates states) const
{
	target.draw(myVertexBuffer.getVertices().data(), myVertexBuffer.getVertices().size(), sf::Triangles, states);
}

void Widget::repaintImpl(bool complexityChanged)
{
	if (!myIsRepaintNeeded || complexityChanged)
	{
		myIsRepaintNeeded = true;

		if (myParent)
		{
			myParent->repaintWidget(*this, complexityChanged);
		}
	}
}

void Widget::onRepaint(Canvas & canvas)
{
	// Subclass drawing code goes here.
}

void Widget::onUpdateVertexBuffer()
{
	// Leave vertex buffer of invisible widgets untouched.
	if (isVisible())
	{
		Canvas canvas(myVertexBuffer, getRenderer());

		// Let widget subclass repaint.
		onRepaint(canvas);

		// Reduce vertex buffer size if necessary. Does nothing if canvas vertex count is same or higher than buffer.
		myVertexBuffer.resizeSection(0, myVertexBuffer.getVertexCount(), canvas.getVertexCount());
	}
}

bool Widget::isRepaintNeeded() const
{
	return myIsRepaintNeeded;
}

bool Widget::isWeakRepaintNeeded() const
{
	return myIsRepaintNeeded || myIsWeakRepaintNeeded;
}

void Widget::repaintWithComplexityUpdate()
{
	repaintImpl(true);
}

void Widget::performRepaint()
{
	onUpdateVertexBuffer();
	myIsRepaintNeeded = false;
}

void Widget::performWeakRepaint()
{
	myIsWeakRepaintNeeded = false;
}

void Widget::updateComplexity()
{
	bool wasComplex = isComplex();

	bool hasComplexPiece = false;
	for (const auto & piece : myPieces)
	{
		if (piece->isComplex())
		{
			hasComplexPiece = true;
			break;
		}
	}

	myHasComplexPiece = hasComplexPiece;

	if (wasComplex != isComplex())
	{
		repaintImpl(true);
	}
}

void Widget::submitQueuedInvocations()
{
	auto app = getParentApplication();

	if (app != nullptr)
	{
		for (auto & invocation : myInvocations)
		{
			if (invocation.queued)
			{
				invocation.queued = false;
				invocation.handle = app->invokeLater(invocation.function, invocation.order);
			}
		}
	}
}

void Widget::runAllInvokeLaterFunctions()
{
	for (auto & invocation : myInvocations)
	{
		if (invocation.queued)
		{
			invocation.function();
		}
		else
		{
			if (!invocation.handle.expired())
			{
				invocation.handle.lock()->invoke();
			}
		}
	}
}

void Widget::disableAllInvokeLaterFunctions()
{
	for (auto & invocation : myInvocations)
	{
		if (!invocation.handle.expired())
		{
			invocation.handle.lock()->remove();
		}
	}
}

void Widget::clearCompletedInvokeLaterFunctions()
{
	auto remIt = std::remove_if(myInvocations.begin(), myInvocations.end(), [](const Invocation & invocation)
	{
		return !invocation.queued && (invocation.handle.expired() || invocation.handle.lock()->isActive());
	});
	myInvocations.erase(remIt, myInvocations.end());
}

}
