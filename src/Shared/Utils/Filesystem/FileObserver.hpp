#ifndef SRC_SHARED_UTILS_FILESYSTEM_FILEOBSERVER_HPP_
#define SRC_SHARED_UTILS_FILESYSTEM_FILEOBSERVER_HPP_

#include <Shared/Utils/Filesystem/DirectoryObserver.hpp>
#include <string>

namespace fs
{

class FileObserver
{
public:
	FileObserver(std::string filename = "");
	~FileObserver();

	void setTargetFile(std::string filename);
	std::string getTargetFile() const;

	void setEventMask(int mask);
	int getEventMask() const;

	bool poll();

private:
	DirectoryObserver directoryObserver;
	std::string target;
};

}

#endif
