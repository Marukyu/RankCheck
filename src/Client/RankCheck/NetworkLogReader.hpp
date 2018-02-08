#ifndef SRC_CLIENT_RANKCHECK_NETWORKLOGREADER_HPP_
#define SRC_CLIENT_RANKCHECK_NETWORKLOGREADER_HPP_

#include <Client/RankCheck/LogReader.hpp>
#include <Client/RankCheck/PlayerData.hpp>
#include <SFML/Config.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace cfg
{
class Config;
}

/**
 * Live-reader for "ApplicationNetwork[1-5].log".
 *
 * Detects when game sessions begin/end, as well as the local SteamID.
 */
class NetworkLogReader : public LogReader
{
public:
	NetworkLogReader();
	virtual ~NetworkLogReader();

	using CallbackVoid = std::function<void()>;

	void setCallbackStart(CallbackVoid callback);
	void setCallbackEnd(CallbackVoid callback);

	sf::Uint64 getLocalSteamID() const;
	bool isSessionActive() const;

private:

	virtual void onReopenFile() override;
	virtual void onReopenFileSuccess() override;
	virtual void onProcessLine(const std::string & line, bool withCallback) override;
	virtual std::vector<std::string> getFileNameList() const override;

	static void parseLocalPlayer(const std::string & line, std::function<void(sf::Uint64)> callback);
	static void parseSessionStart(const std::string & line, CallbackVoid callback);
	static void parseSessionEnd(const std::string & line, CallbackVoid callback);

	CallbackVoid callbackStart;
	CallbackVoid callbackEnd;

	struct Connection
	{
		PlayerData::Team team = PlayerData::UnknownTeam;
		sf::IpAddress ip = sf::IpAddress::None;
		sf::Uint64 steamID = 0;
	};

	sf::Uint64 localSteamID = 0;
	bool playing = false;
};

#endif
