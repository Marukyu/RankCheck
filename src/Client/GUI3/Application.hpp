/*
 * Application.hpp
 *
 *  Created on: May 22, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_APPLICATION_HPP_
#define SRC_CLIENT_GUI3_APPLICATION_HPP_

#include <Client/GUI3/Interface.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Utils/Timer.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// THANKS WINDOWS
#undef interface

namespace gui3
{
namespace res
{
class ResourceManager;
}

/**
 * Handle class for prematurely invoking or removing invocation handles.
 */
class InvocationHandle
{
public:
	InvocationHandle(std::function<void()> function, int order);
	~InvocationHandle();

	void invoke();
	void remove();
	bool isActive() const;

private:

	bool disabled;
	std::function<void()> function;
	int order;

	friend class gui3::Application;
};

class Application
{
public:
	Application();
	virtual ~Application();

	/**
	 * Starts the application. (invokes init().)
	 * 
	 * Receives the argument list.
	 * 
	 * Returns the status code (0 = ok, error otherwise).
	 */
	int run(const std::vector<std::string> & args = std::vector<std::string>());

	/**
	 * Opens an interface (system window) and returns a pointer to it.
	 * 
	 * The pointer is valid until the interface is closed. Therefore, it should only be used for initialization.
	 */
	Interface * open();

	/**
	 * Closes an interface and its associated system window. If this is the last open interface, the application will
	 * exit (the call to run() will return).
	 */
	void close(const Interface * interface);

	/**
	 * Returns a list of all open interfaces.
	 */
	std::vector<Interface *> getOpenInterfaces() const;

	/**
	 * Closes all interfaces and exits the application.
	 */
	void exit();

	/**
	 * Changes the application's framerate limit. A value of 0 means that the framerate is unlimited, and a value of -1
	 * means that the framerate is determined by the VSync timing of the first open interface.
	 */
	void setFramerateLimit(float limit);

	/**
	 * Returns the application's framerate limit. (0 = unlimited, -1 = vsync)
	 */
	float getFramerateLimit() const;

	/**
	 * Returns a reference to the application's resource manager.
	 */
	virtual res::ResourceManager & getResourceManager() const = 0;

	/**
	 * Returns a pointer to the specified application texture.
	 * 
	 * A null pointer is returned if the specified page ID does not exist.
	 * 
	 * The main texture has an ID of 0. This texture, by convention, is used for core GUI textures, such as fonts.
	 */
	virtual const sf::Texture * getTexture(std::size_t pageIndex) const;
	const sf::Texture * getMainTexture() const;

	/**
	 * Returns the coordinates of a fully white pixel on the main texture.
	 */
	virtual sf::Vector2f getWhitePixel() const = 0;

	/**
	 * Returns a reference to the configuration instance of this application.
	 */
	virtual cfg::Config & getConfig() const = 0;

	/**
	 * Runs a function after the end of the current frame.
	 *
	 * This allows deleting resources that may still be in use within the stack during the event loop.
	 *
	 * Optionally, the order of execution can be specified.
	 */
	std::weak_ptr<InvocationHandle> invokeLater(std::function<void()> function, int order = 0);

protected:

	/**
	 * Executed when run() is called. Must contain at least one call to open(), otherwise the application will close
	 * immediately. Should return 0 to indicate successful application execution (application finished normally), or 1
	 * to indicate an error during initialization.
	 */
	virtual int init(const std::vector<std::string> & args);

	/**
	 * Called when an exception is thrown inside any interface's process() function.
	 *
	 * By default, the active exception is rethrown (throw;).
	 */
	virtual void handleError(std::exception & exception);

	/**
	 * Virtual function to generate a subclassed interface.
	 */
	virtual Interface * makeInterface();

private:

	/**
	 * Deallocates all resources whose lifetime is tied to a RenderWindow if there is only one open window remaining.
	 *
	 * This is called by Interfaces before their window is closed.
	 */
	void cleanUpWindowResources();

	/**
	 * Runs all queued "invoke later" functions.
	 */
	void runInvokeLaterFunctions();

	// Holds a list of all open interfaces.
	std::vector<std::unique_ptr<Interface> > myInterfaces;

	// Stores the queued "invoke later" function calls for this frame.
	std::vector<std::shared_ptr<InvocationHandle> > myInvocationHandles;

	// Manages the timestep for this application.
	FramerateTimer myFramerateTimer;

	// The application's framerate limit.
	float myFramerateLimit = 0;

	friend class Interface;
};

}

#endif
