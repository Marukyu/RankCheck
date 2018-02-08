#ifndef SRC_CLIENT_RANKCHECK_LOGREADER_HPP_
#define SRC_CLIENT_RANKCHECK_LOGREADER_HPP_

#include <SFML/Config.hpp>
#include <Shared/Utils/Filesystem/FileObserver.hpp>
#include <Shared/Utils/Timer.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace cfg
{
class Config;
}

/**
 * Abstract superclass for live-readers of log files.
 */
class LogReader
{
public:
	LogReader();
	virtual ~LogReader();

	void initWithConfig(const cfg::Config & config);
	void process();
	void reopenFile(bool initial);

protected:

	std::size_t tell();
	bool seek(std::size_t pos);
	bool seekFromEnd(std::size_t pos);
	bool readInitial();

private:

	virtual void onReopenFile() = 0;
	virtual void onReopenFileSuccess() = 0;
	virtual void onProcessLine(const std::string & line, bool withCallback) = 0;
	virtual std::vector<std::string> getFileNameList() const = 0;
	virtual bool onInitRead();
	virtual void onInitConfig(const cfg::Config & config);

	std::ifstream file;
	std::string openFileName;
	fs::FileObserver observer;
	bool valid = false;
	Timer readTimer;
	sf::Uint64 lastKnownFilesize = 0;
};


#endif
