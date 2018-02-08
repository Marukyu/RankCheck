/*
 * Widget.hpp
 *
 *  Created on: Mar 2, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_WIDGET_HPP_
#define SRC_CLIENT_GUI3_WIDGET_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/KeyEvent.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Piece.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Utils/VertexBuffer.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace cfg
{
class Config;
}

namespace sf
{
class RenderTarget;
class RenderStates;
}

namespace gui3
{

class Canvas;
class Renderer;
class Container;
class Interface;
class Application;
class InvocationHandle;

/**
 * Base class for widgets.
 */
class Widget
{
public:

	/**
	 * Default constructor. Creates an unparented widget.
	 */
	Widget();

	/**
	 * Widgets are non-copyable.
	 */
	Widget(const Widget & widget) = delete;
	Widget & operator=(const Widget & widget) = delete;

	/**
	 * Widgets are movable.
	 */
	Widget(Widget && widget) = delete;
	Widget & operator=(Widget && widget) = delete;

	/**
	 * Virtual destructor.
	 */
	virtual ~Widget();

	/**
	 * Sets the widget's position to the specified value.
	 */
	void setPosition(sf::Vector2f pos);
	void setPosition(float x, float y);

	/**
	 * Returns the widget's position.
	 */
	sf::Vector2f getPosition() const;

	/**
	 * Sets the widget's size to the specified value.
	 */
	void setSize(sf::Vector2f size);
	void setSize(float width, float height);

	/**
	 * Returns the widget's size.
	 */
	sf::Vector2f getSize() const;

	/**
	 * Sets the widget's position and size to the specified rectangle.
	 */
	void setRect(sf::FloatRect rect);
	void setRect(float x, float y, float width, float height);

	/**
	 * Returns the widget's axis-aligned bounding box.
	 */
	sf::FloatRect getRect() const;

	/**
	 * Returns the widget's untranslated axis-aligned bounding box.
	 */
	sf::FloatRect getBaseRect() const;

	/**
	 * Sets the custom transformation to apply to the widget in addition to its default transformation.
	 */
	void setCustomTransform(sf::Transform transform);

	/**
	 * Returns the custom transformation applied to the widget.
	 */
	sf::Transform getCustomTransform() const;

	/**
	 * Returns true if the specified position (from the widget's top left corner) is part of the widget's clickable
	 * area.
	 */
	virtual bool testMouseOver(sf::Vector2f pos) const;

	/**
	 * Changes the widget's visibility. This also affects whether the widget can be interacted with.
	 */
	void setVisible(bool visible);

	/**
	 * Returns the widget's visibility.
	 */
	bool isVisible() const;

	/**
	 * Changes the widget's usability.
	 */
	void setEnabled(bool enabled);

	/**
	 * Returns the widget's usability.
	 */
	bool isEnabled() const;

	/**
	 * Focuses the widget locally. This will not steal focus if any of the widget's parents are not currently focused.
	 */
	void acquireFocus();

	/**
	 * Unfocuses the widget locally, resulting in no widget being focused.
	 */
	void dropFocus();

	/**
	 * Focuses the widget globally. This will steal the global focus by changing the entire parent chain's focus.
	 */
	void stealFocus();

	/**
	 * Checks if the widget and its entire parent chain is focused.
	 */
	bool isFocused() const;

	/**
	 * Changes whether the widget can be focused.
	 */
	void setFocusable(bool focusable);

	/**
	 * Returns whether the widget can be focused.
	 */
	bool isFocusable() const;

	/**
	 * Changes the widget's Z-Position. A higher number means that the widget is in front of others.
	 */
	void setZPosition(int zpos);

	/**
	 * Returns the widget's Z-position.
	 */
	int getZPosition() const;

	/**
	 * Changes the widget's rendering order among other widgets with the same Z-Position.
	 */
	void sendToFront();
	void sendToBack();

	/**
	 * Changes the widget's renderer. Pass a null pointer to inherit renderer from parent widget.
	 */
	void setRendererOverride(const Renderer * renderer);

	/**
	 * Returns the widget's renderer if it is overridden, null pointer otherwise.
	 */
	const Renderer * getRendererOverride() const;

	/**
	 * Returns the widget's applied renderer. Returns a null pointer if no renderer is assigned to the widget or any of
	 * its parents.
	 */
	const Renderer * getRenderer() const;

	/**
	 * Changes the mouse cursor image resource to be drawn while the mouse cursor is hovering over this widget.
	 * Pass a null pointer to specify no cursor override.
	 */
	void setMouseCursorOverride(Ptr<res::Image> cursor);

	/**
	 * Returns the widget's mouse cursor image if it is overridden, null pointer otherwise.
	 */
	Ptr<res::Image> getMouseCursorOverride() const;

	/**
	 * Adds a callback for mouse events with the specified filter and order (lower = earlier).
	 * 
	 * The specified function will be called and supplied with a MouseEvent parameter everytime a mouse event occurs.
	 * For more details, see the MouseEvent class.
	 * 
	 * Returns an optional ID for future callback removal.
	 */
	CallbackHandle<MouseEvent> addMouseCallback(EventFunc<MouseEvent> func, int typeFilter, int order = 0);

	/**
	 * Adds a callback for keyboard events with the specified filter and order (lower = earlier).
	 */
	CallbackHandle<KeyEvent> addKeyboardCallback(EventFunc<KeyEvent> func, int typeFilter, int order = 0);

	/**
	 * Adds a callback for state change events with the specified filter and order (lower = earlier).
	 */
	CallbackHandle<StateEvent> addStateCallback(EventFunc<StateEvent> func, int typeFilter, int order = 0);

	/**
	 * Registers a function to be called every tick with the specified order (lower = earlier).
	 * 
	 * This only fires as long as the widget has a valid parent.
	 */
	CallbackHandle<> addTickCallback(EventFunc<> func, int order = 0);

	/**
	 * Notifies the widget of a mouse event.
	 */
	void fireMouseEvent(MouseEvent event);

	/**
	 * Notifies the widget of a keyboard event.
	 */
	void fireKeyboardEvent(KeyEvent event);

	/**
	 * Notifies the widget of a state change event.
	 */
	void fireStateEvent(StateEvent event);

	/**
	 * Runs the widget's tick function.
	 */
	void fireTick();

	/**
	 * Invalidates the widget's vertices, causing the widget to be redrawn in the current frame.
	 */
	void repaint();

	/**
	 * Invalidates the widget's contained vertices, causing the widget to be redrawn in the current frame without
	 * recalculating the vertices internally.
	 */
	void repaintWeak();

	/**
	 * Returns the widget's transformation matrix. Normally holds a single translation by the widget position.
	 */
	const sf::Transform & getTransform() const;

	/**
	 * Returns the widget's inverse transformation matrix.
	 */
	const sf::Transform & getInverseTransform() const;

	/**
	 * Retruns true if the widget can be rendered using the GUI's main texture and does not require special rendering.
	 */
	bool isComplex() const;

	/**
	 * Returns true if the widget listens for key events at all times, even when un-focused.
	 */
	bool isGlobalKeyListener() const;

	/**
	 * Returns a pointer to the widget's parent container. If the widget has no parent, a null pointer is returned.
	 */
	Container * getParent() const;

	/**
	 * Returns a pointer to the widget's parent interface, or a null pointer if the widget is orphaned.
	 */
	virtual Interface * getParentInterface() const;

	/**
	 * Returns a pointer to the widget's parent application, or a null pointer if the widget is orphaned.
	 */
	Application * getParentApplication() const;

	/**
	 * Runs a function after the end of the current frame.
	 *
	 * This allows deleting resources that may still be in use within the stack during the event loop.
	 *
	 * Optionally, the order of execution can be specified.
	 */
	void invokeLater(std::function<void()> function, int order = 0);

protected:

	template<typename T, typename ... Args>
	T * addPiece(Args&&... args)
	{
		auto uniquePtr = makeUnique<T>(std::forward<Args>(args...)...);
		auto ptr = uniquePtr.get();
		appendPiece(std::move(uniquePtr));
		return ptr;
	}

	/**
	 * Changes whether the widget is forced to render complexly.
	 */
	void setComplexOverride(bool complex);

	/**
	 * Checks whether the widget is forced to render complexly.
	 */
	bool isComplexOverride() const;

	/**
	 * Sets the custom internal transformation to apply to the widget in addition to its default transformation.
	 */
	void setInternalTransform(sf::Transform transform);

	/**
	 * Returns the custom internal transformation applied to the widget.
	 */
	sf::Transform getInternalTransform() const;

	/**
	 * Changes whether this widget listens for key events at all times, even when unfocused.
	 */
	void setGlobalKeyListener(bool globalKeyListener);

	/**
	 * Checks if the specified widget is within this widget's parent chain.
	 */
	bool isRecursiveParent(const Widget & widget) const;

	/**
	 * Provides direct access the widget's vertex buffer.
	 */
	VertexBuffer & getVertexBuffer();
	const VertexBuffer & getVertexBuffer() const;

	/**
	 * Convenience/shortcut functions.
	 */
	cfg::Config & config() const;
	res::ResourceManager & resources() const;

private:

	/**
	 * Changes the widget's parent, which is the container the widget is placed inside of.
	 */
	void setParent(Container * parent);

	/**
	 * Adds a piece to the widget.
	 */
	void appendPiece(std::unique_ptr<Piece> piece);

	/**
	 * Marks the widget's transform as invalid.
	 */
	void invalidateTransform();

	/**
	 * Invalidates the widget's vertices, causing the widget to be redrawn in the current frame.
	 * 
	 * Optionally updates complexity.
	 */
	void repaintImpl(bool complexityChanged);

	/**
	 * Called after vertex invalidation. This function should be overwritten to render the widget to the provided
	 * canvas.
	 */
	virtual void onRepaint(Canvas & canvas);

	/**
	 * Called every frame for drawing if the widget is complex.
	 */
	virtual void onRender(sf::RenderTarget & target, sf::RenderStates states) const;

	/**
	 * Called after vertex invalidation for both primitive and complex widgets. Calls onRepaint() with a Canvas object
	 * and uses it to update the vertex buffer by default.
	 * 
	 * This function should only be used if Canvas does not give you the required flexibility.
	 */
	virtual void onUpdateVertexBuffer();

	/**
	 * Returns true if the widget needs to be repainted.
	 */
	bool isRepaintNeeded() const;

	/**
	 * Returns true if the widget needs to be weakly repainted.
	 */
	bool isWeakRepaintNeeded() const;

	/**
	 * Repaints the widget and notifies the container of the widget's complexity.
	 */
	void repaintWithComplexityUpdate();

	/**
	 * Repaints the widget.
	 */
	void performRepaint();

	/**
	 * Unsets the weak repaint reqiurement flag.
	 */
	void performWeakRepaint();

	/**
	 * Updates the rendering complexity of this widget based on whether a complex component is contained.
	 */
	void updateComplexity();

	/**
	 * Calls "Application::invokeLater" for all queued invocation calls.
	 */
	void submitQueuedInvocations();

	/**
	 * Runs all "invoke later" functions.
	 */
	void runAllInvokeLaterFunctions();

	/**
	 * Disables all "invoke later" functions.
	 */
	void disableAllInvokeLaterFunctions();

	/**
	 * Removes all completed "invoke later" functions.
	 */
	void clearCompletedInvokeLaterFunctions();

	/**
	 * The widget's position (originating at the top left corner), from the top left corner of the container.
	 */
	sf::Vector2f myPosition;

	/**
	 * The widget's size.
	 */
	sf::Vector2f mySize;

	/**
	 * The widget's custom transformation matrix; for internal use by widget.
	 */
	std::unique_ptr<sf::Transform> myInternalTransform;

	/**
	 * The widget's custom transformation matrix; for customization by user.
	 */
	std::unique_ptr<sf::Transform> myCustomTransform;

	/**
	 * Holds the widget's pieces.
	 */
	std::vector<std::unique_ptr<Piece> > myPieces;

	/**
	 * The widget's cached transformation matrix, and its inverse.
	 */
	mutable sf::Transform myTransform;
	mutable sf::Transform myInverseTransform;

	/**
	 * True if the widget's [inverse] transformation matrix is invalid and needs to be recalculated.
	 */
	mutable bool myIsTransformInvalid;
	mutable bool myIsInverseTransformInvalid;

	/**
	 * The widget's visibility (whether the widget should be rendered or not; also affects usability).
	 */
	bool myIsVisible;

	/**
	 * The widget's usability (whether the widget should be interactable or not).
	 */
	bool myIsEnabled;

	/**
	 * The widget's focusability (whether the widget can be focused or not).
	 */
	bool myIsFocusable;

	/**
	 * The widget's Z-Position.
	 */
	int myZPosition;

	/**
	 * The widget's renderer override.
	 */
	const Renderer * myRendererOverride;

	/**
	 * The widget's mouse cursor override.
	 */
	Ptr<res::Image> myCursorOverride;

	/**
	 * Queued or stored invocation.
	 */
	struct Invocation
	{
		bool queued;
		std::function<void()> function;
		int order;
		std::weak_ptr<InvocationHandle> handle;
	};

	/**
	 * The widget's queued or stored "invokeLater" calls.
	 */
	std::vector<Invocation> myInvocations;

	/**
	 * Callback lists for registering event listeners.
	 */
	CallbackManager<MouseEvent> myMouseCallbacks;
	CallbackManager<KeyEvent> myKeyCallbacks;
	CallbackManager<StateEvent> myStateCallbacks;
	CallbackManager<> myTickCallbacks;

	/**
	 * Weak pointer to the container holding this widget. Null pointer if the widget is uncontained.
	 */
	Container * myParent;

	/**
	 * True if the widget will be repainted in the current frame.
	 */
	bool myIsRepaintNeeded;

	/**
	 * True if the widget will be repainted by the container in the current frame.
	 */
	bool myIsWeakRepaintNeeded;

	/**
	 * True if the widget is primitively rendered (i.e. can be rendered using only default-textured vertices).
	 */
	bool myIsComplex;
	bool myHasComplexPiece;

	/**
	 * True if the widget receives key events from the application, regardless of whether the widget is focused or not.
	 */
	bool myIsGlobalKeyListener;

	/**
	 * The vertex buffer holding the widget's vertices.
	 */
	VertexBuffer myVertexBuffer;

	friend class Interface;
	friend class Container;
	friend class Piece;
};

}

#endif
