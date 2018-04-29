#include <Poco/File.h>
#include <Poco/Path.h>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <fstream>
#include <iostream>
#include <string>

static bool logOpen = false;
static std::ofstream logFile;

static sf::Clock debugClock;

void writeDebugOutput(std::string log)
{
	if (!logOpen)
	{
		Poco::Path dir(Poco::Path::dataHome());
		dir.pushDirectory("rankcheck");
		try {
			Poco::File(dir).createDirectories();
		} catch (const std::exception &e) {
			std::cerr << "Cannot create log directory at " << dir.toString() 
				<< ": " << e.what() << std::endl;
			return;
		}

		logFile.open(dir.setFileName("error.log").toString());
		logOpen = true;
	}
	logFile << log << std::endl;
}

void writeDebugLocation(const char* fileName, unsigned int lineNumber, const char* functionName)
{
	writeDebugOutput(
		"[" + cNtoS(debugClock.getElapsedTime().asMicroseconds()) + "] " + fileName + cNtoS(lineNumber) + functionName);
}
