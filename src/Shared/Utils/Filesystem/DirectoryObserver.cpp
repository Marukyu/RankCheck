#include <Poco/AbstractDelegate.h>
#include <Poco/AbstractEvent.h>
#include <Poco/BasicEvent.h>
#include <Poco/Delegate.h>
#include <Poco/DirectoryWatcher.h>
#include <Poco/File.h>
#include <SFML/System/Lock.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/Filesystem/DirectoryObserver.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <algorithm>
#include <exception>

namespace fs
{

class WatcherImpl
{
public:

	WatcherImpl(DirectoryObserver & observer, std::string directory, int eventMask) :
		observer(observer),
		watcher(directory, convertEventMaskToPoco(eventMask))
	{
		watcher.itemAdded += Poco::Delegate<WatcherImpl, const Poco::DirectoryWatcher::DirectoryEvent>(this,
			&WatcherImpl::handleEvent);
		watcher.itemModified += Poco::Delegate<WatcherImpl, const Poco::DirectoryWatcher::DirectoryEvent>(this,
			&WatcherImpl::handleEvent);
		watcher.itemRemoved += Poco::Delegate<WatcherImpl, const Poco::DirectoryWatcher::DirectoryEvent>(this,
			&WatcherImpl::handleEvent);
		watcher.itemMovedFrom += Poco::Delegate<WatcherImpl, const Poco::DirectoryWatcher::DirectoryEvent>(this,
			&WatcherImpl::handleEvent);
		watcher.itemMovedTo += Poco::Delegate<WatcherImpl, const Poco::DirectoryWatcher::DirectoryEvent>(this,
			&WatcherImpl::handleEvent);
	}

	~WatcherImpl()
	{
		watcher.itemAdded.clear();
		watcher.itemModified.clear();
		watcher.itemRemoved.clear();
		watcher.itemMovedFrom.clear();
		watcher.itemMovedTo.clear();
	}

	void handleEvent(const void *, const Poco::DirectoryWatcher::DirectoryEvent & event)
	{
		DirectoryObserver::Event eventToSend;
		eventToSend.filename = event.item.path();
		eventToSend.type = convertEventMaskFromPoco(event.event);

		sf::Lock lock(observer.eventMutex);
		observer.events.push(eventToSend);
	}

	static int convertEventMaskToPoco(int eventMask)
	{
		// Conveniently chosen enum values.
		return eventMask;
	}

	static DirectoryObserver::Event::Type convertEventMaskFromPoco(int eventMask)
	{
		return (DirectoryObserver::Event::Type) eventMask;
	}

private:

	DirectoryObserver & observer;
	Poco::DirectoryWatcher watcher;
};

DirectoryObserver::DirectoryObserver() :
	watching(true),
	impl(nullptr),
	events(),
	watchedDirectory(),
	eventMask(Event::All)
{
}

DirectoryObserver::~DirectoryObserver()
{
}

bool DirectoryObserver::pollEvent(Event& event)
{
	sf::Lock lock(eventMutex);
	if (events.empty())
	{
		return false;
	}
	event = std::move(events.front());
	events.pop();
	return true;
}

void DirectoryObserver::setEventMask(int eventMask)
{
	if (this->eventMask != eventMask)
	{
		this->eventMask = eventMask;
		updateSettings();
	}
}

int DirectoryObserver::getEventMask() const
{
	return eventMask;
}

void DirectoryObserver::setWatchedDirectory(std::string watchedDirectory)
{
	if (this->watchedDirectory != watchedDirectory)
	{
		this->watchedDirectory = watchedDirectory;
		updateSettings();
	}
}

std::string DirectoryObserver::getWatchedDirectory() const
{
	return watchedDirectory;
}

void DirectoryObserver::startWatching()
{
	if (!isWatching())
	{
		if (watchedDirectory.empty() || eventMask == 0)
		{
			impl = nullptr;
		}
		else
		{
			try
			{
				impl = makeUnique<WatcherImpl>(*this, getWatchedDirectory(), getEventMask());
			}
			catch (std::exception & ex)
			{
				impl = nullptr;
			}
		}
		watching = true;
	}
}

void DirectoryObserver::stopWatching()
{
	impl = nullptr;
	watching = false;
}

bool DirectoryObserver::isWatching() const
{
	return watching;
}

void DirectoryObserver::updateSettings()
{
	if (isWatching())
	{
		stopWatching();
		startWatching();
	}
}

}
