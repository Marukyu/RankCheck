#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <fstream>
#include <string>

static bool logOpen = false;
static std::ofstream logFile;

static sf::Clock debugClock;

void writeDebugOutput(std::string log)
{
	if (!logOpen)
	{
		logOpen = true;
		logFile.open("rankcheck-error-log.txt");
	}
	logFile << log << std::endl;
}

void writeDebugLocation(const char* fileName, unsigned int lineNumber, const char* functionName)
{
	writeDebugOutput(
		"[" + cNtoS(debugClock.getElapsedTime().asMicroseconds()) + "] " + fileName + cNtoS(lineNumber) + functionName);
}
