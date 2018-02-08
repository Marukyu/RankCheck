#include <Client/RankCheck/LeagueReader.hpp>
#include <Client/RankCheck/RankChecker.hpp>
#include <SFML/Network/Http.hpp>
#include <Shared/Config/DataTypes.hpp>
#include <Shared/Config/JSONConfig.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <iterator>
#include <utility>

RankChecker::RankChecker()
{
	localSteamID = 0;
}

RankChecker::~RankChecker()
{
	for (const auto & conn : connections)
	{
		conn->thread.join();
	}
}

void RankChecker::setLocalSteamID(sf::Uint64 localSteamID)
{
	this->localSteamID = localSteamID;
}

void RankChecker::setHost(std::string host, unsigned short port)
{
	this->host = host;
	this->port = port;
}

void RankChecker::setUriParameters(std::string prefix, std::string separator, std::string suffix)
{
	uriPrefix = prefix;
	uriSeparator = separator;
	uriSuffix = suffix;
}

void RankChecker::addSteamIDRequest(sf::Uint64 steamID, std::function<void(Result)> callback)
{
	pendingRequests.push_back({steamID, callback});
}

void RankChecker::sendRequestIfNeeded()
{
	if (!pendingRequests.empty())
	{
		sendRequest([](InfoResult){});
	}
}

void RankChecker::sendRequest(InfoCallback callback)
{
	std::unique_ptr<Connection> conn = makeUnique<Connection>();
	for (auto & req : pendingRequests)
	{
		conn->callbacks[req.steamID].push_back(std::move(req.callback));
	}
	pendingRequests.clear();

	Connection * connPtr = conn.get();
	conn->thread = std::thread([this,connPtr]()
	{
		sf::Http http;
		http.setHost(host, port);

		std::string queryString;
		bool firstQueryStringPart = true;
		for (const auto & queryStringPart : connPtr->callbacks)
		{
			if (firstQueryStringPart)
			{
				firstQueryStringPart = false;
			}
			else
			{
				queryString += uriSeparator;
			}
			queryString += cNtoS(queryStringPart.first);
		}

		sf::Http::Request request;
		request.setUri(uriPrefix + queryString + uriSuffix);

		sf::Http::Response response = http.sendRequest(request);

		connPtr->result = response.getBody();
		connPtr->done = true;
	});

	connections.push_back(std::move(conn));
}

void RankChecker::process()
{
	for (auto it = connections.begin(); it != connections.end();)
	{
		Connection & connection = **it;
		if (connection.done)
		{
			ParseResult result = parseJSON(connection.result);
			for (const auto & callback : connection.callbacks)
			{
				Result res;
				if (!result.success)
				{
					res.code = Result::NotFound;
				}
				else
				{
					auto it2 = result.entries.find(callback.first);
					if (it2 == result.entries.end())
					{
						res.code = Result::NotFound;
					}
					else
					{
						res = it2->second;
						res.code = Result::Success;
					}
				}
				for (const auto & cb : callback.second)
				{
					cb(res);
				}
			}
			connection.thread.detach();
			it = connections.erase(it);
		}
		else
		{
			++it;
		}
	}
}

RankChecker::ParseResult RankChecker::parseJSON(const std::string& json)
{
	cfg::JSONConfig config;
	try
	{
		config.loadFromString(json);
	}
	catch (std::exception & e)
	{
		debug() << e.what();
		return ParseResult();
	}

	auto getInt = [&config](const std::string & key)
	{
		return cStoI(config.readValue(key).content);
	};
	auto getULong = [&config](const std::string & key)
	{
		return cStoUL(config.readValue(key).content);
	};

	ParseResult result;

	result.success = true;
	result.info.entryCount = getULong("totalEntryCount");
	result.info.lastUpdateTime = getULong("lastUpdate");

	LeagueReader::getInstance().setEntryCount(result.info.entryCount);

	for (std::size_t i = 0; i < getULong("entries.length"); ++i)
	{
		auto getEntryInt = [&](const std::string & key)
		{
			return getInt("entries[" + cNtoS(i) + "]." + key);
		};
		auto getEntryULong = [&](const std::string & key)
		{
			return getULong("entries[" + cNtoS(i) + "]." + key);
		};

		Result entry;
		entry.code = Result::Success;
		entry.scoredata = std::vector<sf::Int32>(14, -1);
		entry.steamID = getEntryULong("steamid");
		entry.scoredata[0] = getEntryInt("rank");
		entry.scoredata[1] = getEntryInt("rating");
		entry.scoredata[3] = getEntryInt("totalWins");
		entry.scoredata[4] = getEntryInt("totalLosses");
		entry.scoredata[8] = getEntryInt("mainNaut");
		entry.scoredata[9] = getEntryInt("seasonWins");
		entry.scoredata[10] = getEntryInt("seasonLosses");
		result.entries.emplace(entry.steamID, std::move(entry));
	}

	return result;
}
