#include <Poco/File.h>
#include <Poco/Path.h>
#include <Shared/Utils/Filesystem/FileObserver.hpp>

namespace fs
{

FileObserver::FileObserver(std::string filename)
{
	setEventMask(DirectoryObserver::Event::All);
	setTargetFile(filename);
}

FileObserver::~FileObserver()
{
}

void FileObserver::setTargetFile(std::string filename)
{
	directoryObserver.stopWatching();

	if (!filename.empty())
	{
		// Convert filename to absolute path.
		Poco::Path filePath(filename);
		filePath.makeAbsolute();

		// Assign path relative to working directory.
		target = filePath.toString();

		// Set event mask and target directory.
		directoryObserver.setWatchedDirectory(filePath.parent().toString());

		// Observe directory that contains the file.
		directoryObserver.startWatching();
	}
}

std::string FileObserver::getTargetFile() const
{
	return target;
}

void FileObserver::setEventMask(int mask)
{
	directoryObserver.setEventMask(mask);
}

int FileObserver::getEventMask() const
{
	return directoryObserver.getEventMask();
}

bool FileObserver::poll()
{
	DirectoryObserver::Event event;

	while (directoryObserver.pollEvent(event))
	{
		if (event.filename == target)
		{
			return true;
		}
	}
	return false;
}

}
