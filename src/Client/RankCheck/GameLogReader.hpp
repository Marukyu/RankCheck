#ifndef SRC_CLIENT_RANKCHECK_GAMELOGREADER_HPP_
#define SRC_CLIENT_RANKCHECK_GAMELOGREADER_HPP_

#include <Client/RankCheck/LogReader.hpp>
#include <functional>
#include <string>
#include <vector>

class GameLogReader : public LogReader
{
public:
	GameLogReader();
	virtual ~GameLogReader();

	bool isInGame() const;

private:

	virtual void onReopenFile() override;
	virtual void onReopenFileSuccess() override;
	virtual void onProcessLine(const std::string & line, bool withCallback) override;
	virtual std::vector<std::string> getFileNameList() const override;

	static void parseGameStart(const std::string & line, std::function<void()> callback);
	static void parseGameEnd(const std::string & line, std::function<void()> callback);

	bool inGame;
};

#endif
