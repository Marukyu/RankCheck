#ifndef SRC_SHARED_UTILS_FILESYSTEM_DIRECTORYOBSERVER_HPP_
#define SRC_SHARED_UTILS_FILESYSTEM_DIRECTORYOBSERVER_HPP_

#include <SFML/System/Mutex.hpp>
#include <memory>
#include <queue>
#include <string>

namespace fs
{

class WatcherImpl;

class DirectoryObserver
{
public:
	DirectoryObserver();
	~DirectoryObserver();

	struct Event
	{
		enum Type
		{
			Added = 1 << 0,
			Removed = 1 << 1,
			Modified = 1 << 2,
			MovedFrom = 1 << 3,
			MovedTo = 1 << 4,

			All = (1 << 5) - 1
		};

		Type type;
		std::string filename;
	};

	bool pollEvent(Event & event);

	void setEventMask(int eventMask);
	int getEventMask() const;

	void setWatchedDirectory(std::string watchedDirectory);
	std::string getWatchedDirectory() const;

	void startWatching();
	void stopWatching();
	bool isWatching() const;

private:

	void updateSettings();

	bool watching;
	std::unique_ptr<WatcherImpl> impl;
	std::queue<Event> events;
	sf::Mutex eventMutex;

	std::string watchedDirectory;
	int eventMask;

	friend class WatcherImpl;
};

}

#endif
