#include <Client/RankCheck/GameFolder.hpp>
#include <Client/RankCheck/LogReader.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <fstream>

static cfg::Float diskReadInterval("rankcheck.diskReadInterval");

LogReader::LogReader()
{
	observer.setEventMask(fs::DirectoryObserver::Event::Added);
}

LogReader::~LogReader()
{
}

void LogReader::initWithConfig(const cfg::Config& config)
{
	onInitConfig(config);

	std::string gameFolder = GameFolder::getInstance().getFolder();

	if (!file.is_open())
	{
		for (const std::string & filename : getFileNameList())
		{
			openFileName = gameFolder + "/" + filename;
			reopenFile(true);
			if (file.good())
			{
				valid = true;
				break;
			}
		}

		if (openFileName.empty())
		{
			debug() << "Failed to open " << (getFileNameList().empty() ? "log file" : getFileNameList().back())
				<< "!";
			valid = false;
		}
	}

	sf::Time timerInterval = sf::seconds(config.get(diskReadInterval));
	if (readTimer.getTime() != timerInterval)
	{
		readTimer.setTime(timerInterval);
	}
}

void LogReader::process()
{
	if (!valid)
	{
		return;
	}

	if (observer.poll())
	{
		reopenFile(false);
	}

	if (readTimer.tick())
	{
		file.clear();
		auto currentFilePos = file.tellg();
		file.seekg(0, std::ios::end);
		sf::Uint64 currentFileSize = file.tellg();

		if (lastKnownFilesize > currentFileSize)
		{
			debug() << openFileName << " is smaller than it used to be, reopening...";
			reopenFile(false);
			lastKnownFilesize = 0;
		}
		else
		{
			lastKnownFilesize = currentFileSize;
			file.seekg(currentFilePos);
		}

		if (file.is_open())
		{
			std::string line;
			while (std::getline(file, line))
			{
				onProcessLine(line, true);
			}
		}
		else
		{
			reopenFile(false);
		}
	}
}

void LogReader::reopenFile(bool initial)
{
	onReopenFile();

	debug() << "Opening " << openFileName << " for reading";
	file.open(openFileName);
	observer.setTargetFile(openFileName);

	if (file.good())
	{
		if (onInitRead())
		{
			readInitial();
		}
	}
	else
	{
		debug() << "Warning: " << openFileName << " could not be read!";
		if (initial)
		{
			observer.setTargetFile("");
			openFileName.clear();
		}
		file.close();
	}
}

std::size_t LogReader::tell()
{
	file.clear();
	return file.tellg();
}

bool LogReader::onInitRead()
{
	return true;
}

bool LogReader::seek(std::size_t pos)
{
	file.clear();
	file.seekg(pos);
	return file.good();
}

bool LogReader::seekFromEnd(std::size_t pos)
{
	file.clear();
	file.seekg(-sf::Int64(pos), std::ios::end);
	return file.good();
}

bool LogReader::readInitial()
{
	std::string line;
	while (std::getline(file, line))
	{
		onProcessLine(line, false);
	}

	if (!file.eof())
	{
		file.close();
		return false;
	}
	else
	{
		file.clear();
		onReopenFileSuccess();
		return true;
	}
}

void LogReader::onInitConfig(const cfg::Config& config)
{
}
