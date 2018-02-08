/*
 * Container.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#include <Client/Graphics/UtilitiesSf.hpp>
#include <Client/GUI3/Container.hpp>
#include <Client/GUI3/Events/KeyEvent.hpp>
#include <Client/GUI3/Utils/VertexBuffer.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <typeinfo>

namespace gui3
{

std::vector<Widget*> Container::getContainedWidgets() const
{
	std::vector<Widget *> ret;
	ret.reserve(myWidgets.size());

	for (const WidgetData & widget : myWidgets)
	{
		if (widget.widget)
		{
			ret.push_back(widget.widget);
		}
	}

	return ret;
}

bool Container::isWidgetContained(const Widget& widget) const
{
	return findWidget(widget) != InvalidID;
}

Widget* Container::getMouseOverWidget() const
{
	return myMouseOverWidget;
}

Container::~Container()
{
	clearWidgets();
}

Container::Container() :
	myMouseMonitor(*this),
	myMouseOverWidget(nullptr),
	myFocusedWidget(nullptr),
	myComplexWidgetCount(0),
	myIsClippingWidgets(true),
	myIsGlobalRepaintNeeded(true),
	myIsForcedComplex(false)
{
	// State event forwarding.
	addStateCallback([this](StateEvent event)
	{
		handleStateEvent(event);
	}, StateEvent::Any, 0);

	addMouseCallback([this](MouseEvent event)
	{
		handleMouseEvent(event);
	}, MouseEvent::Any, 0);

	addKeyboardCallback([this](KeyEvent event)
	{
		handleKeyEvent(event);
	}, KeyEvent::Any, 0);

	addTickCallback([this]()
	{
		handleTickEvent();
	}, 0);

}

bool Container::addWidget(Widget& widget)
{
	if (isWidgetContained(widget))
	{
		return false;
	}

	// Disallow adding widgets from this container's parent chain.
	if (isRecursiveParent(widget))
	{
		return false;
	}

	// Remove widget from old parent, if necessary.
	if (widget.getParent())
	{
		widget.getParent()->removeWidget(widget);
	}

	WidgetData data;
	data.widget = &widget;
	data.vertexCount = 0;
	data.isComplex = false;
	data.callback = widget.addStateCallback([this,&widget](StateEvent event)
	{
		handleWidgetStateEvent(widget, event);
	}, StateEvent::Any);

	myWidgets.invalidate();
	myWidgets.push_back(std::move(data));

	widget.setParent(this);
	widget.repaintWithComplexityUpdate();

	// Arrange widget into correct array position.
	updateWidgetZPositionImpl(widget, widget.getZPosition(), true, true);

	return true;
}

bool Container::removeWidget(Widget& widget)
{
	std::size_t widgetIndex = findWidget(widget);

	if (widgetIndex == InvalidID)
	{
		return false;
	}

	if (myMouseOverWidget == &widget)
	{
		myMouseOverWidget = nullptr;
	}

	for (auto & mouseDownWidget : myMouseDownWidgets)
	{
		if (mouseDownWidget == &widget)
		{
			mouseDownWidget = nullptr;
		}
	}

	if (myFocusedWidget == &widget)
	{
		myFocusedWidget = nullptr;
	}

	updateWidgetComplexity(widgetIndex, false);

	getVertexBuffer().resizeSection(getWidgetVertexBufferOffset(widget), getWidgetVertexBufferSize(widget), 0);

	myWidgets[widgetIndex].callback.remove();
	myWidgets.invalidate();
	myWidgets.erase(myWidgets.begin() + widgetIndex);

	updateMouseover();

	widget.setParent(nullptr);
	return true;
}

void Container::clearWidgets()
{
	while (!myWidgets.empty())
	{
		removeWidget(*myWidgets.back().widget);
	}
}

void Container::repaintWidget(Widget& widget, bool complexityChanged)
{
	if (complexityChanged)
	{
		std::size_t widgetIndex = findWidget(widget);

		if (widgetIndex == InvalidID)
		{
			return;
		}

		updateWidgetComplexity(widgetIndex, widget.isComplex());
	}

	repaint();
}

sf::FloatRect Container::getContainerBoundingBox() const
{
	return getBaseRect();
}

sf::FloatRect Container::getContainerClipBox() const
{
	return getBaseRect();
}

void Container::setClippingWidgets(bool clipping)
{
	if (myIsClippingWidgets != clipping)
	{
		myIsClippingWidgets = clipping;

		myIsGlobalRepaintNeeded = true;
		repaint();
	}
}

bool Container::isClippingWidgets() const
{
	return myIsClippingWidgets;
}

const std::size_t Container::InvalidID = std::numeric_limits<std::size_t>::max();

std::size_t Container::findWidget(const Widget & widget) const
{
	for (std::size_t i = 0; i < myWidgets.size(); ++i)
	{
		if (myWidgets[i].widget == &widget)
		{
			return i;
		}
	}

	return InvalidID;
}

void Container::setForcedComplex(bool complex)
{
	if (myIsForcedComplex != complex)
	{
		myIsForcedComplex = complex;

		if (myIsForcedComplex)
		{
			myIsGlobalRepaintNeeded = true;
		}

		updateContainerComplexity();
		repaint();
	}
}

bool Container::isForcedComplex() const
{
	return myIsForcedComplex;
}

bool Container::isBuffering() const
{
	return myIsForcedComplex;
}

std::size_t Container::getWidgetVertexBufferOffset(const Widget& widget) const
{
	std::size_t vertexCount = 0;

	for (std::size_t i = 0; i < myWidgets.size(); ++i)
	{
		if (myWidgets[i].widget == &widget)
		{
			return vertexCount;
		}

		vertexCount += myWidgets[i].vertexCount;
	}

	return 0;
}

std::size_t Container::getWidgetVertexBufferSize(const Widget& widget) const
{
	std::size_t widgetIndex = findWidget(widget);

	if (widgetIndex == InvalidID)
	{
		return 0;
	}

	return myWidgets[widgetIndex].vertexCount;
}

void Container::setFocusedWidget(Widget* widget)
{
	if (widget == myFocusedWidget)
	{
		return;
	}

	Widget* oldFocusedWidget = myFocusedWidget;
	myFocusedWidget = widget;

	// Only forward focus events if the current container is focused.
	if (isFocused())
	{
		if (oldFocusedWidget != nullptr)
		{
			oldFocusedWidget->fireStateEvent(StateEvent::FocusLost);
		}
		if (myFocusedWidget != nullptr)
		{
			myFocusedWidget->fireStateEvent(StateEvent::FocusGained);
		}
	}
}

Widget* Container::getFocusedWidget() const
{
	return myFocusedWidget;
}

void Container::updateWidgetZPosition(Widget& widget, int zpos)
{
	updateWidgetZPositionImpl(widget, zpos, true, false);
}

void Container::sendWidgetToFront(Widget& widget)
{
	// This automatically sends the widget to the top-most position within the group.
	updateWidgetZPositionImpl(widget, widget.getZPosition(), true, false);
}

void Container::sendWidgetToBack(Widget& widget)
{
	// This automatically sends the widget to the bottom-most position within the group.
	updateWidgetZPositionImpl(widget, widget.getZPosition(), false, false);
}

void Container::updateWidgetZPositionImpl(Widget& widget, int zpos, bool upperBound, bool skipLast)
{
	// Comparator for upper_bound (target zpos and widget data).
	auto zComparatorUpper = [](int left, const WidgetData & right)
	{
		return left < right.widget->getZPosition();
	};

	// Comparator for lower_bound (widget data and target zpos).
	auto zComparatorLower = [](const WidgetData & left, int right)
	{
		return left.widget->getZPosition() < right;
	};

	// Get current location of the widget to be moved.
	std::size_t oldLocation = findWidget(widget);

	// Ignore non-contained widgets.
	if (oldLocation == InvalidID)
	{
		return;
	}

	// This will hold the target location. It will be reduced by 1 if higher than the source location, as moveWidgets()
	// requires two valid widget indices.
	std::size_t newLocation;

	// Upper limit for range to search through.
	auto searchEnd = myWidgets.end();

	// Allow last widget to be skipped (for sorting newly added widgets that are placed at the end of array).
	if (skipLast)
	{
		--searchEnd;
	}

	// Choose search method based on whether the widget should be at the top or bottom of group.
	// At this point, the widget is still correctly sorted, but zpos already contains the target position.
	if (upperBound)
	{
		// Binary search for topmost location.
		auto newIt = std::upper_bound(myWidgets.begin(), searchEnd, zpos, zComparatorUpper);
		newLocation = newIt - myWidgets.begin();
	}
	else
	{
		// Binary search for bottommost location.
		auto newIt = std::lower_bound(myWidgets.begin(), searchEnd, zpos, zComparatorLower);
		newLocation = newIt - myWidgets.begin();
	}

	// Perform widget rotation.
	moveWidget(oldLocation, newLocation > oldLocation ? newLocation - 1 : newLocation);
}

void Container::moveWidget(std::size_t extractIndex, std::size_t insertIndex)
{
	if (extractIndex == insertIndex)
	{
		return;
	}

	if (extractIndex >= myWidgets.size() || insertIndex >= myWidgets.size())
	{
		throw Error("Invalid widget move");
	}

	// Get vertex count at extraction index.
	std::size_t vertexCount = myWidgets[extractIndex].vertexCount;

	// Check move direction.
	if (extractIndex < insertIndex)
	{
		// Right move.

		// Widgets without vertices do not require vertex buffer reordering.
		if (vertexCount != 0)
		{
			// Count vertices to section start.
			std::size_t vertexStart = getWidgetVertexBufferOffset(*myWidgets[extractIndex].widget);

			// Count vertices to section end.
			std::size_t vertexEnd = getWidgetVertexBufferOffset(*myWidgets[insertIndex].widget);

			// Add vertex count at section end.
			vertexEnd += myWidgets[insertIndex].vertexCount;

			// Rotate vertices.
			myVertexBuffer.rotateSection(vertexStart, vertexStart + vertexCount, vertexEnd);
		}

		// Rotate widgets (new start should be right neighbor of extracted widget).
		std::rotate(myWidgets.begin() + extractIndex, myWidgets.begin() + extractIndex + 1,
			myWidgets.begin() + insertIndex + 1);
	}
	else if (extractIndex > insertIndex)
	{
		// Left move.

		// Widgets without vertices do not require vertex buffer reordering.
		if (vertexCount != 0)
		{
			// Count vertices to section start.
			std::size_t vertexStart = getWidgetVertexBufferOffset(*myWidgets[insertIndex].widget);

			// Count vertices to section end.
			std::size_t vertexEnd = getWidgetVertexBufferOffset(*myWidgets[extractIndex].widget);

			// Rotate vertices.
			myVertexBuffer.rotateSection(vertexStart, vertexEnd, vertexEnd + vertexCount);
		}

		// Rotate widgets (new start should be be extracted widget).
		std::rotate(myWidgets.begin() + insertIndex, myWidgets.begin() + extractIndex,
			myWidgets.begin() + extractIndex + 1);
	}

	// Recalculate mouse-overed widget.
	updateMouseover();

	// Repaint container.
	repaint();
}

void Container::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::Moved:
	default:
		// Ignore.
		break;

	case StateEvent::Resized:
		invalidateContainerBounds();
		break;

	case StateEvent::FocusGained:
	case StateEvent::FocusLost:
		// Container focus was changed externally.

		// Forward focus change event to locally focused widget.
		if (getFocusedWidget() != nullptr)
		{
			getFocusedWidget()->fireStateEvent(event);
		}

		break;

	case StateEvent::ResourcesChanged:
	case StateEvent::ParentApplicationChanged:
	case StateEvent::ConfigChanged:
		// Forward resource, parent application or configuration change to all widgets.
		// Use a locking iterator in case the event handler adds or removes widgets to/from this container.
		for (auto it = myWidgets.lbegin(); it != myWidgets.lend(); ++it)
		{
			it->widget->fireStateEvent(event);
		}
		break;

	case StateEvent::RendererChanged:

		// Forward renderer change to all widgets that inherit this value from their parent.
		// Use a locking iterator in case the event handler adds or removes widgets to/from this container.
		for (auto it = myWidgets.lbegin(); it != myWidgets.lend(); ++it)
		{
			if (it->widget->getRendererOverride() == nullptr)
			{
				it->widget->fireStateEvent(event);
			}
		}
		break;
	}
}

void Container::handleMouseEvent(MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::Enter:
	case MouseEvent::Click:
	default:
		// Ignore.
		break;

	case MouseEvent::Leave:
	{
		// If a widget is currently mouse-overed, un-mouseover it.
		if (myMouseOverWidget != nullptr)
		{
			// Send leave event to widget.
			myMouseOverWidget->fireMouseEvent(
				MouseEvent::generatePositionalEvent(MouseEvent::Leave,
					myMouseOverWidget->getInverseTransform().transformPoint(event.position)));

			// Change mouse-overed widget.
			myMouseOverWidget = nullptr;
		}
	}
		break;

	case MouseEvent::ButtonDown:
	case MouseEvent::ButtonUp:
	case MouseEvent::Move:
	case MouseEvent::ScrollX:
	case MouseEvent::ScrollY:
	{
		// Current mouse position in container coordinates.
		sf::Vector2f mousePos = event.position;

		// Hold mouse overed widget and its transformed mouse event position.
		Widget * mouseOverWidget = nullptr;
		sf::Vector2f widgetTransformedPos;

		// Iterate over widgets in reverse rendering order.
		for (auto it = myWidgets.rbegin(); it != myWidgets.rend(); ++it)
		{
			// Transform mouse position according to widget transformation.
			widgetTransformedPos = it->widget->getInverseTransform().transformPoint(mousePos);

			if (it->widget->isVisible() && it->widget->testMouseOver(widgetTransformedPos))
			{
				mouseOverWidget = it->widget;
				break;
			}
		}

		// Check if a mouse button was released (this gets special treatment).
		if (event.type == MouseEvent::ButtonUp)
		{
			// Check if a widget had this mouse button held down on it.
			Widget * mouseDownWidget = getMouseDownWidget(event.button);

			if (mouseDownWidget != nullptr)
			{
				// Send mouse button release event.
				mouseDownWidget->fireMouseEvent(
					MouseEvent::generateButtonEvent(MouseEvent::ButtonUp,
						mouseDownWidget->getInverseTransform().transformPoint(mousePos), event.button));

				// Send click event if the mouse cursor is still on the widget.
				if (mouseDownWidget == mouseOverWidget)
				{
					mouseDownWidget->fireMouseEvent(
						MouseEvent::generateButtonEvent(MouseEvent::Click,
							mouseDownWidget->getInverseTransform().transformPoint(mousePos), event.button));
				}

				// Unset mouse down widget.
				setMouseDownWidget(event.button, nullptr);
			}
		}
		else
		{
			// Check if the mouse cursor moved over another widget.
			bool isMouseOverWidgetChanged = (event.type == MouseEvent::Move && myMouseOverWidget != mouseOverWidget);

			// Check if mouse cursor left the previously mouse-overed widget.
			if (isMouseOverWidgetChanged)
			{
				// Only leave old widget if it exists.
				if (myMouseOverWidget != nullptr)
				{
					// Leave old widget.
					myMouseOverWidget->fireMouseEvent(
						MouseEvent::generatePositionalEvent(MouseEvent::Leave,
							myMouseOverWidget->getInverseTransform().transformPoint(mousePos)));
				}

				// Change mouse-overed widget.
				myMouseOverWidget = mouseOverWidget;
			}

			// Check if the mouse cursor is now over a widget.
			if (mouseOverWidget != nullptr)
			{
				// Check if a mouse button was pressed.
				if (event.type == MouseEvent::ButtonDown)
				{
					// Remember this widget as the one having the mouse button pressed on it.
					setMouseDownWidget(event.button, mouseOverWidget);

					// Check if the click was done using the left mouse button.
					if (event.button == MouseEvent::Left)
					{
						// Move the clicked widget to the front.
						mouseOverWidget->sendToFront();

						// Focus the clicked widget.
						mouseOverWidget->acquireFocus();
					}
				}

				// Forward mouse event.
				mouseOverWidget->fireMouseEvent(MouseEvent::generateRepositionedEvent(event, widgetTransformedPos));

				// Check if mouse cursor just entered a new widget.
				if (isMouseOverWidgetChanged)
				{
					// Enter new widget.
					mouseOverWidget->fireMouseEvent(
						MouseEvent::generatePositionalEvent(MouseEvent::Enter, widgetTransformedPos));
				}
			}

			// Forward mouse movements to non-mouse-overed widgets if any mouse button is held on them.
			if (event.type == MouseEvent::Move)
			{
				// Loop over all widgets that have a mouse button held on them.
				for (Widget * mouseDownWidget : myMouseDownWidgets)
				{
					// Ignore null entries and the mouse-overed widget (since it already received the move event).
					if (mouseDownWidget != nullptr && mouseDownWidget != mouseOverWidget)
					{
						// Forward mouse event.
						mouseDownWidget->fireMouseEvent(
							MouseEvent::generateRepositionedEvent(event,
								mouseDownWidget->getInverseTransform().transformPoint(mousePos)));
					}
				}
			}
		}
	}

		break;
	}
}

void Container::handleKeyEvent(KeyEvent event)
{
	// Forward key event to locally focused widget.
	if (getFocusedWidget() != nullptr)
	{
		getFocusedWidget()->fireKeyboardEvent(event);
	}
}

void Container::handleTickEvent()
{
	// Forward tick event to all contained widgets.
	// Use a locking iterator in case the event handler adds or removes widgets to/from this container.
	// TODO: Implement this more efficiently by only forwarding tick events to widgets that need them.
	for (auto it = myWidgets.lbegin(); it != myWidgets.lend(); ++it)
	{
		it->widget->fireTick();
	}
}

void Container::handleWidgetStateEvent(Widget& widget, StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::Moved:
	case StateEvent::Resized:
	case StateEvent::CustomTransformChanged:
	case StateEvent::VisibilityChanged:
		// TODO: perhaps add MouseMaskChanged event
		updateMouseover();
		break;

	default:
		break;
	}
}

void Container::setMouseDownWidget(MouseEvent::Button mouseButton, Widget* widget)
{
	if (widget == nullptr)
	{
		if (mouseButton >= myMouseDownWidgets.size())
		{
			return;
		}
	}
	else
	{
		if (widget->getParent() != this)
		{
			return;
		}

		while (myMouseDownWidgets.size() <= mouseButton)
		{
			myMouseDownWidgets.push_back(nullptr);
		}
	}

	myMouseDownWidgets[mouseButton] = widget;

	// TODO: Is it really necessary to remove empty mouse down entries from the back of the vector?
	if (widget == nullptr)
	{
		while (!myMouseDownWidgets.empty() && myMouseDownWidgets.back() == nullptr)
		{
			myMouseDownWidgets.pop_back();
		}
	}
}

Widget* Container::getMouseDownWidget(MouseEvent::Button mouseButton) const
{
	if (mouseButton < myMouseDownWidgets.size())
	{
		return myMouseDownWidgets[mouseButton];
	}
	else
	{
		return nullptr;
	}
}

void Container::invalidateContainerBounds()
{
	// Use a locking iterator in case the event handler adds or removes widgets to/from this container.
	for (auto it = myWidgets.lbegin(); it != myWidgets.lend(); ++it)
	{
		it->widget->fireStateEvent(StateEvent(StateEvent::ParentBoundsChanged));
	}
}

void Container::updateMouseover()
{
	if (myMouseMonitor.isMouseOver() || myMouseMonitor.isMouseDownAnyButton())
	{
		MouseEvent event = MouseEvent::generatePositionalEvent(MouseEvent::Move, myMouseMonitor.getMousePosition());
		handleMouseEvent(event);
	}
}

void Container::updateWidgetComplexity(std::size_t widgetIndex, bool complex)
{
	if (myWidgets[widgetIndex].isComplex && !complex)
	{
		myWidgets[widgetIndex].isComplex = false;
		myComplexWidgetCount--;
	}
	else if (!myWidgets[widgetIndex].isComplex && complex)
	{
		myWidgets[widgetIndex].isComplex = true;
		myComplexWidgetCount++;
	}

	updateContainerComplexity();
}

void Container::updateContainerComplexity()
{
	setComplexOverride(isBuffering() || myComplexWidgetCount != 0);
}

void Container::onUpdateVertexBuffer()
{
	if (!isVisible())
	{
		return;
	}

	// Keep track of current vertex section.
	std::size_t sectionStart = 0;

	// Create buffer for widget clipping.
	std::vector<sf::Vertex> clipBuffer;

	// Loop over all widgets.
	for (WidgetData & widgetData : myWidgets)
	{
		Widget & widget = *widgetData.widget;

		// Check if the current widget needs a weak/full repaint.
		bool widgetRepaintNeeded = widget.isRepaintNeeded();
		bool widgetWeakRepaintNeeded = widget.isWeakRepaintNeeded();

		// Ignore global repaint flag for performRepaint call: the vertices in the widget are still valid.
		if (widgetRepaintNeeded)
		{
			// Repaint the widget.
			widget.performRepaint();
		}

		// Process the vertices in the widget's vertex buffer.
		if (widgetWeakRepaintNeeded || myIsGlobalRepaintNeeded)
		{
			// Mark widget as weakly repainted.
			widget.performWeakRepaint();

			// Check if the widget is invisible (nothing), primitive (basic vertex data only) or complex (custom
			// RenderStates).
			if (!widget.isVisible())
			{
				// Remove invisible widget's vertices entirely.
				getVertexBuffer().resizeSection(sectionStart, widgetData.vertexCount, 0);

				// Update vertex count cache.
				widgetData.vertexCount = 0;
			}
			else if (!widget.isComplex())
			{
				// Get widget-specific transformation matrix.
				sf::Transform transform = widget.getTransform();

				// Keep track of current widget vertex count.
				std::size_t vertexCount = widget.getVertexBuffer().getVertexCount();

				// Check if widget vertices need to be clipped.
				if (isClippingWidgets())
				{
					// Prepare buffer for out-of-place clipping.
					clipBuffer = widget.getVertexBuffer().getVertices();

					// Transform vertices.
					for (sf::Vertex & vertex : clipBuffer)
					{
						vertex.position = transform.transformPoint(vertex.position);
					}

					// Clip vertices.
					clipVertices(clipBuffer, getContainerClipBox());

					// Update vertex count.
					vertexCount = clipBuffer.size();

					// Update vertex buffer size and capacity.
					getVertexBuffer().resizeSection(sectionStart, widgetData.vertexCount, vertexCount);

					// Replace vertices in buffer.
					getVertexBuffer().replaceSection(sectionStart, clipBuffer.begin(), clipBuffer.end());
				}
				else
				{
					// Update vertex buffer size and capacity.
					getVertexBuffer().resizeSection(sectionStart, widgetData.vertexCount, vertexCount);

					// Replace vertices in buffer.
					getVertexBuffer().replaceSection(sectionStart, widget.getVertexBuffer().getVertices().begin(),
						widget.getVertexBuffer().getVertices().end());

					// Transform vertices.
					getVertexBuffer().transformSection(sectionStart, vertexCount, transform);
				}

				// Update vertex count cache.
				widgetData.vertexCount = vertexCount;
			}
			else
			{
				// Complex widget: the widget itself does the rendering, delete all local vertices (if there are any).
				getVertexBuffer().resizeSection(sectionStart, widgetData.vertexCount, 0);

				// Set vertex and scrap count for this widget to 0.
				widgetData.vertexCount = 0;
			}
		}

		// Increment current section start.
		sectionStart += widgetData.vertexCount;
	}

	myIsGlobalRepaintNeeded = false;
}

void Container::onRender(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Keep track of beginning and end of consecutive primitive-renderable widget sections.
	std::size_t sectionStart = 0;
	std::size_t sectionEnd = 0;

	// Check if any complex widgets have to be drawn.
	// TODO: Extract function "Container::hasComplexWidgets()".
	if (myComplexWidgetCount != 0)
	{
		// Keep track of separate RenderStates and clipping rectangle for complex widgets.
		sf::RenderStates complexWidgetStates = states;
		sf::FloatRect clipRect;

		// Only initialize clipping rectangle if clipping is enabled.
		if (isClippingWidgets())
		{
			clipRect = states.transform.transformRect(getContainerClipBox());
		}

		// Loop over all widgets.
		for (const WidgetData & widgetData : myWidgets)
		{
			Widget & widget = *widgetData.widget;

			// Skip invisible widgets entirely.
			if (!widget.isVisible())
			{
				continue;
			}

			if (!widget.isComplex())
			{
				// Skip over vertices for now, they will be drawn all at once as soon as the next non-primitive widget
				// or the end of the buffer is encountered.
				sectionEnd += widgetData.vertexCount;
			}
			else
			{
				// Draw currently accumulated vertices.
				target.draw(getVertexBuffer().getVertices().data() + sectionStart, sectionEnd - sectionStart,
					sf::Triangles, states);

				// Move section start marker.
				sectionStart = sectionEnd;

				// Calculate widget-specific transformation matrix.
				complexWidgetStates.transform = states.transform * widget.getTransform();

				// Determine whether widget should be clipped or not.
				if (isClippingWidgets())
				{
					// Render complex widget within clipping rectangle.
					drawClipped(DrawableWrapper([&widget](sf::RenderTarget & target, sf::RenderStates states)
					{
						widget.onRender(target, states);
					}), target, complexWidgetStates, clipRect);
				}
				else
				{
					// Render complex widget.
					widget.onRender(target, complexWidgetStates);
				}
			}
		}
	}
	else
	{
		// Skip to end immediately.
		sectionEnd = getVertexBuffer().getVertexCount();
	}

	// Draw final remaining vertices.
	target.draw(getVertexBuffer().getVertices().data() + sectionStart, sectionEnd - sectionStart, sf::Triangles,
		states);
}

}
