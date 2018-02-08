#include <Client/RankCheck/LeagueReader.hpp>
#include <Client/RankCheck/NetworkLogReader.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/Filesystem/DirectoryObserver.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <cstddef>
#include <utility>

NetworkLogReader::NetworkLogReader()
{
	callbackStart = []{};
	callbackEnd = []{};
}

NetworkLogReader::~NetworkLogReader()
{
}

void NetworkLogReader::setCallbackStart(CallbackVoid callback)
{
	this->callbackStart = callback;
}

void NetworkLogReader::setCallbackEnd(CallbackVoid callback)
{
	this->callbackEnd = callback;
}

sf::Uint64 NetworkLogReader::getLocalSteamID() const
{
	return localSteamID;
}

void NetworkLogReader::onProcessLine(const std::string & line, bool withCallback)
{
	parseLocalPlayer(line, [=](sf::Uint64 steamID)
	{
		if (localSteamID == 0)
		{
			debug() << "Found local SteamID: " << steamID;
			localSteamID = steamID;
		}
	});

	parseSessionStart(line, [=]
	{
		playing = true;
		debug() << "Game session started.";
		if (withCallback)
		{
			callbackStart();
		}
	});

	parseSessionEnd(line, [=]
	{
		playing = false;
		debug() << "Game session ended.";
		if (withCallback)
		{
			callbackEnd();
		}
	});
}

void NetworkLogReader::parseLocalPlayer(const std::string& line, std::function<void(sf::Uint64)> callback)
{
	static const std::string playerJoinKey = "Succesfuly Received Steam user stats  ";

	std::size_t found = line.find(playerJoinKey);
	if (found != std::string::npos)
	{
		sf::Uint64 steamID = cStoUL(line.substr(found + playerJoinKey.size()));
		if (steamID != 0)
		{
			callback(steamID);
		}
	}
}

void NetworkLogReader::parseSessionStart(const std::string & line, CallbackVoid callback)
{
	static const std::string newMatchmakingJoinKey = "changed:  Match  ";
	static const std::string newMatchmakingJoinSuffix = "  joining  to  joined";

	if (stringEndsWith(line, newMatchmakingJoinSuffix) && line.find(newMatchmakingJoinKey) != std::string::npos)
	{
		callback();
	}
}

void NetworkLogReader::parseSessionEnd(const std::string & line, CallbackVoid callback)
{
	static const std::string newMatchmakingLeaveKey = "changed:  Match  ";
	static const std::string newMatchmakingLeaveSuffix = "  to  idle";

	if (stringEndsWith(line, newMatchmakingLeaveSuffix) && line.find(newMatchmakingLeaveKey) != std::string::npos)
	{
		callback();
	}
}

void NetworkLogReader::onReopenFile()
{
	localSteamID = 0;
}

void NetworkLogReader::onReopenFileSuccess()
{
}

std::vector<std::string> NetworkLogReader::getFileNameList() const
{
	std::vector<std::string> names;
	for (int i = 5; i > 0; --i)
	{
		names.push_back("ApplicationNetwork" + cNtoS(i) + ".log");
	}
	return names;
}

bool NetworkLogReader::isSessionActive() const
{
	return playing;
}
