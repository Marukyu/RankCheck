/*
 * Interface.hpp
 *
 *  Created on: Jun 15, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_INTERFACE_HPP_
#define SRC_CLIENT_GUI3_INTERFACE_HPP_

#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>

namespace gui3
{

class Application;

/**
 * Manages a system window that processes and renders GUI3 widgets.
 * Has a single container that automatically resizes to fit the interface.
 * Can only be used within the context of an application.
 */
class Interface
{
public:

	virtual ~Interface();

	/**
	 * Returns the root container that holds this interface's widgets.
	 */
	Panel & getRootContainer();
	const Panel & getRootContainer() const;

	/**
	 * Returns the application this interface belongs to.
	 */
	Application & getParentApplication() const;

	/**
	 * Sets the interface's window title.
	 */
	void setTitle(sf::String title);

	/**
	 * Returns the interface's window title.
	 */
	sf::String getTitle() const;

	/**
	 * Resizes the window to the specified size (window mode) or switches to the closest available resolution
	 * (fullscreen mode).
	 */
	void resize(sf::Vector2u size, bool fullscreen);

	/**
	 * Returns this interface's window position.
	 */
	sf::Vector2i getPosition() const;

	/**
	 * Returns this interface's window size or screen resolution.
	 */
	sf::Vector2u getSize() const;

	/**
	 * Returns true if the interface is in fullscreen mode.
	 */
	bool isFullscreen() const;

	/**
	 * Returns true if the interface is currently focused.
	 */
	bool isFocused() const;

	/**
	 * Shows or hides the system mouse cursor on this interface.
	 */
	void setUseSystemCursor(bool visible);

	/**
	 * Returns the system mouse cursor's visibility state on this interface.
	 */
	bool isUsingSystemCursor() const;

	/**
	 * Changes the window's icon.
	 */
	void setIcon(const sf::Image & icon);

protected:

	/**
	 * Constructs a new default interface. Only usable by Application.
	 */
	Interface(Application * parentApplication);

	/**
	 * Returns a RenderTarget corresponding to this interface's window.
	 */
	sf::RenderTarget & getRenderTarget();
	const sf::RenderTarget & getRenderTarget() const;

	/**
	 * These functions are called by the controlling application to manage the interface's lifetime.
	 */
	bool isWindowOpen() const;
	void openWindow();
	void closeWindow();

private:

	/**
	 * Processes events and renders widgets.
	 */
	void process();

	/**
	 * Called for each event during processing.
	 */
	void processEvent(const sf::Event & event);

	/**
	 * Called for each event during processing, in addition to built-in event handling.
	 */
	virtual void onEvent(const sf::Event & event);

	/**
	 * Called after rendering, but before myWindow.display().
	 */
	virtual void onRender();

	/**
	 * Container subclass for a Container immediately attached to an Interface.
	 */
	class RootContainer : public Panel
	{
	public:

		RootContainer(Interface * parentInterface);

		virtual ~RootContainer();

		/**
		 * Returns the interface this root container is attached to.
		 */
		Interface * getParentInterface() const override;

	private:

		Interface * myParentInterface;
	};

	/**
	 * Returns the closest videomode to the specified size vector.
	 */
	static sf::VideoMode findGoodVideoMode(sf::Vector2u compare);

	/**
	 * The interface's window.
	 */
	sf::RenderWindow myWindow;

	/**
	 * Various (self-explanatory) flags.
	 */
	bool myIsFullscreen;
	bool myIsMouseCursorVisible;
	bool myHasFocus;

	/**
	 * Root container widget that holds this interface's widgets.
	 */
	RootContainer myRootContainer;

	/**
	 * Parent application.
	 */
	Application * myParentApplication;

	/**
	 * Window position/size.
	 */
	sf::Vector2i myWindowPosition;
	sf::Vector2u myWindowSize;

	/**
	 * Window title.
	 */
	sf::String myWindowTitle;

	friend class Application;

};

} // namespace gui3

#endif
