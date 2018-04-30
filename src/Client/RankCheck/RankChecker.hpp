#ifndef SRC_CLIENT_RANKCHECK_RANKCHECKER_HPP_
#define SRC_CLIENT_RANKCHECK_RANKCHECKER_HPP_

#include <SFML/Config.hpp>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

class RankChecker
{
public:

	using RequestID = sf::Uint64;

	struct Result
	{
		enum Code
		{
			Success,
			Timeout,
			NotFound,
			Error
		};

		sf::Uint64 steamID;
		std::string username;
		std::vector<sf::Int32> scoredata;
		Code code;
	};

	struct InfoResult
	{
		sf::Uint64 entryCount;
		sf::Uint64 lastUpdateTime;
	};

	using EntryCallback = std::function<void(Result)>;
	using InfoCallback = std::function<void(InfoResult)>;

	RankChecker();
	virtual ~RankChecker();

	void setHost(std::string host, unsigned short port);
	void setUriParameters(std::string prefix, std::string separator, std::string suffix);
	void setLocalSteamID(sf::Uint64 localSteamID);

	void addSteamIDRequest(sf::Uint64 steamID, EntryCallback callback);
	void sendRequestIfNeeded();
	void sendRequest(InfoCallback callback);

	void process();

private:

	struct ParseResult
	{
		bool success = false;
		InfoResult info;
		std::map<sf::Uint64, Result> entries;
	};

	static ParseResult parseJSON(const std::string & json);

	sf::Uint64 localSteamID;
	std::string host;
	std::string uriPrefix;
	std::string uriSeparator;
	std::string uriSuffix;
	unsigned short port;

	struct PendingRequest
	{
		sf::Uint64 steamID;
		EntryCallback callback;
	};

	struct Connection
	{
		std::atomic_bool done = ATOMIC_VAR_INIT(false);
		std::thread thread;
		std::string result;
		std::map<sf::Uint64, std::vector<EntryCallback> > callbacks;
	};

	std::vector<PendingRequest> pendingRequests;
	std::vector<std::unique_ptr<Connection> > connections;
};

#endif
