/*
 * Container.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_CONTAINER_HPP_
#define SRC_CLIENT_GUI3_CONTAINER_HPP_

#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Utils/MouseMonitor.hpp>
#include <Client/GUI3/Widget.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <Shared/Utils/LockableVector.hpp>
#include <cstddef>
#include <vector>

namespace gui3
{

/**
 * A widget that contains other widgets.
 * 
 * Note for subclassing: Containers are not rendered by themselves. Use child widgets to render backgrounds.
 */
class Container : public Widget
{
public:

	/**
	 * Destroys the container. Removes all contained widgets.
	 */
	virtual ~Container();

	/**
	 * Returns a vector containing pointers to all widgets within the container.
	 */
	std::vector<Widget *> getContainedWidgets() const;

	/**
	 * Returns true if the specified widget is contained, false otherwise.
	 */
	bool isWidgetContained(const Widget & widget) const;

	/**
	 * Returns the rectangle to which widgets are confined, which is the the container's base bounding box (rect at 0,0
	 * with size of container).
	 */
	sf::FloatRect getContainerBoundingBox() const;

	/**
	 * Returns the widget the mouse cursor is currently hovering over. Returns a null pointer if no widget is hovered
	 * within the container.
	 */
	Widget * getMouseOverWidget() const;

protected:

	/**
	 * Creates a container without any held widgets.
	 */
	Container();

	/**
	 * Adds the specified widget to the container. This removes the widget from its previous parent.
	 * It will be automatically removed from the container once the widget's lifetime ends.
	 * 
	 * Returns true if the widget was successfully added, false if the widget is already contained in this container.
	 */
	bool addWidget(Widget & widget);

	/**
	 * Removes the specified widget from the container. If the widget is not contained within the container, nothing
	 * happens.
	 * 
	 * Returns true if the widget was successfully removed, false if the widget was not contained in this container.
	 */
	bool removeWidget(Widget & widget);

	/**
	 * Removes all currently held widgets from the container.
	 */
	void clearWidgets();

	/**
	 * Notifies the container that the specified widget has to be repainted.
	 */
	void repaintWidget(Widget & widget, bool complexityChanged);

	/**
	 * True if the container should always be complex-rendered. This prevents the container from sending its widgets'
	 * vertices to the parent container.
	 * 
	 * The container will behave as if this option is enabled if one or more of the contained widgets is non-primitive.
	 */
	void setForcedComplex(bool complex);

	/**
	 * True if complex rendering is force-enabled on this container, false otherwise.
	 */
	bool isForcedComplex() const;

	/**
	 * True if the container performs buffering of its widgets' vertices, false if buffering is delegated to the parent
	 * container.
	 */
	bool isBuffering() const;

	/**
	 * Returns the rectangle to which widgets are clipped, in relation to the container's transformed position.
	 * Affects rendering and event processing; applied after the widgets have been transformed by the container
	 * transformation.
	 * 
	 * Defaults to the container's base bounding box (rect at 0,0 with size of container).
	 */
	sf::FloatRect getContainerClipBox() const;

	/**
	 * Sets whether the container should clip its widgets to the container bounds during rendering.
	 */
	void setClippingWidgets(bool clipping);

	/**
	 * Returns true if the container should clip its widgets to the container bounds during rendering, and false
	 * otherwise.
	 * 
	 * Defaults to true.
	 */
	bool isClippingWidgets() const;

private:

	static const std::size_t InvalidID;

	/**
	 * Returns the index of the specified widget.
	 */
	std::size_t findWidget(const Widget & widget) const;

	/**
	 * Returns the specified widget's vertex buffer offset.
	 */
	std::size_t getWidgetVertexBufferOffset(const Widget & widget) const;

	/**
	 * Returns the specified widget's vertex buffer size.
	 */
	std::size_t getWidgetVertexBufferSize(const Widget & widget) const;

	/**
	 * Focus-related functions.
	 */
	void setFocusedWidget(Widget * widget);
	Widget * getFocusedWidget() const;

	/**
	 * Reorders the widget to correct its index with respect to Z-Position.
	 * 
	 * This function should be called before the Z-Position is changed, with the new Z-Position as the parameter.
	 */
	void updateWidgetZPosition(Widget & widget, int zpos);

	/**
	 * Reorders the widget within its current equal Z-Position group.
	 */
	void sendWidgetToFront(Widget & widget);
	void sendWidgetToBack(Widget & widget);

	/**
	 * Reorders the widget to correct its index with respect to Z-Position.
	 * 
	 * May skip the last widget in the list (for sorting newly added widgets) and may use lower bound instead of upper.
	 */
	void updateWidgetZPositionImpl(Widget & widget, int zpos, bool upperBound, bool skipLast);

	/**
	 * Moves the widget at the specified index to the specified target index.
	 */
	void moveWidget(std::size_t extractIndex, std::size_t insertIndex);

	/**
	 * Various event handlers.
	 */
	void handleStateEvent(StateEvent event);
	void handleMouseEvent(MouseEvent event);
	void handleKeyEvent(KeyEvent event);
	void handleTickEvent();
	void handleWidgetStateEvent(Widget & widget, StateEvent event);

	/**
	 * Changes/returns the widget that is currently being pressed with a specific mouse button.
	 */
	void setMouseDownWidget(MouseEvent::Button mouseButton, Widget * widget);
	Widget * getMouseDownWidget(MouseEvent::Button mouseButton) const;

	/**
	 * Notifies widgets of changes to the container's bounds. Called when the return value of getContainerTransform,
	 * getContainerBoundingBox, getContainerClipBox is changed.
	 */
	void invalidateContainerBounds();

	/**
	 * Performs mouse position related checks and fires mouse movement/enter/leave events.
	 */
	void updateMouseover();

	/**
	 * Checks the specified widget's rendering complexity.
	 */
	void updateWidgetComplexity(std::size_t widgetIndex, bool complex);

	/**
	 * Updates the rendering complexity of this container based on whether a complex widget is contained.
	 */
	void updateContainerComplexity();

	/**
	 * Called after vertex invalidation.
	 */
	void onUpdateVertexBuffer() override;

	/**
	 * Called every frame for drawing if the widget is complex.
	 */
	void onRender(sf::RenderTarget & target, sf::RenderStates states) const override;

	/**
	 * Cached container transform, inverse transform, bounding box and clip box.
	 */
	sf::Transform myContainerTransform;
	sf::Transform myContainerInverseTransform;
	sf::FloatRect myContainerBoundingBox;
	sf::FloatRect myContainerClipBox;

	/**
	 * Structure for per-widget information: holds a pointer to the widget and the number of vertices.
	 */
	struct WidgetData
	{
		Widget * widget;
		std::size_t vertexCount;
		bool isComplex;
		CallbackHandle<StateEvent> callback;
	};

	/**
	 * A vector of pointers and widget-related data for each widget held within this container.
	 * 
	 * It can be locked to prevent modification during iteration, for example widget reparenting during event passing.
	 */
	LockableVector<WidgetData> myWidgets;

	/**
	 * Keeps track of the mouse position.
	 */
	MouseMonitor myMouseMonitor;

	/**
	 * Holds the widget that the mouse cursor is currently on.
	 */
	Widget * myMouseOverWidget;

	/**
	 * Holds all widgets that currently have a mouse button pressed on them. The index is the mouse button that was
	 * pressed on the widget.
	 */
	std::vector<Widget*> myMouseDownWidgets;

	/**
	 * Holds the widget that currently has the container-local focus.
	 */
	Widget * myFocusedWidget;

	/**
	 * Holds the number of complex-rendered widgets within the container.
	 */
	std::size_t myComplexWidgetCount;

	/**
	 * True if the container clips its widgets when rendering.
	 */
	bool myIsClippingWidgets;

	/**
	 * True if all widget's vertices have to be repainted.
	 */
	bool myIsGlobalRepaintNeeded;

	/**
	 * True if this container is self-buffering, managing its own vertex buffer for all the contained widgets.
	 * False if this container delegates vertex buffering to the recursively first buffering parent container.
	 */
	bool myIsForcedComplex;

	friend class Widget;
	friend class Interface;
};

}

#endif
