#include <Client/RankCheck/GameLogReader.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/DebugLog.hpp>

GameLogReader::GameLogReader() :
	inGame(false)
{
}

GameLogReader::~GameLogReader()
{
}

bool GameLogReader::isInGame() const
{
	return inGame;
}

void GameLogReader::onReopenFile()
{
	inGame = false;
}

void GameLogReader::onReopenFileSuccess()
{
}

void GameLogReader::onProcessLine(const std::string& line, bool withCallback)
{
	parseGameStart(line, [=]
	{
		debug() << "Match has started!";
		inGame = true;
	});

	parseGameEnd(line, [=]
	{
		debug() << "Match has ended!";
		inGame = false;
	});
}

std::vector<std::string> GameLogReader::getFileNameList() const
{
	std::vector<std::string> names;
	for (int i = 5; i > 0; --i)
	{
		names.push_back("ApplicationOther" + cNtoS(i) + ".log");
	}
	return names;
}

void GameLogReader::parseGameStart(const std::string& line, std::function<void()> callback)
{
	static const std::string gameStartKey = "\tGameManager constructor starts";

	if (line.find(gameStartKey) != std::string::npos)
	{
		callback();
	}
}

void GameLogReader::parseGameEnd(const std::string& line, std::function<void()> callback)
{
	static const std::string gameEndKey = "\tGameManager destructor starts";

	if (line.find(gameEndKey) != std::string::npos)
	{
		callback();
	}
}
