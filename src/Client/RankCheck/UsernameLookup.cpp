#include <Client/RankCheck/LeagueReader.hpp>
#include <Client/RankCheck/UsernameLookup.hpp>
#include <SFML/Network/Http.hpp>
#include <pugixml.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Config/DataTypes.hpp>
#include <Shared/Config/JSONConfig.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <algorithm>
#include <exception>
#include <iterator>
#include <utility>

UsernameLookup::UsernameLookup()
{
}

UsernameLookup::~UsernameLookup()
{
	for (const auto & conn : connections)
	{
		conn->thread.join();
	}
}

void UsernameLookup::setHost(std::string host, unsigned short port)
{
	this->host = host;
	this->port = port;
}

void UsernameLookup::setUriParameters(std::string prefix, std::string suffix)
{
	uriPrefix = prefix;
	uriSuffix = suffix;
}

void UsernameLookup::lookup(sf::Uint64 steamID, Callback callback)
{
	auto cacheEntry = cache.find(steamID);
	if (cacheEntry != cache.end())
	{
		if (!cacheEntry->second.timer.expired())
		{
			callback(cacheEntry->second.name);
			return;
		}
	}

	std::unique_ptr<Connection> conn = makeUnique<Connection>();
	conn->callback = callback;
	conn->steamID = steamID;
	Connection * connPtr = conn.get();

	conn->thread = std::thread([this,connPtr]()
	{
		sf::Http http;
		http.setHost(host, port);

		sf::Http::Request request;
		request.setUri(uriPrefix + cNtoS(connPtr->steamID) + uriSuffix);

		sf::Http::Response response = http.sendRequest(request);
		connPtr->result = parseResponse(response.getBody());
		connPtr->done = true;
	});

	connections.push_back(std::move(conn));
}

void UsernameLookup::process()
{
	for (auto it = connections.begin(); it != connections.end();)
	{
		Connection & connection = **it;
		if (connection.done)
		{
			Countdown countdown;
			countdown.restart(sf::seconds(300));
			cache.insert(std::make_pair(connection.steamID, CacheEntry { connection.result, countdown }));
			connection.callback(connection.result);
			connection.thread.detach();
			it = connections.erase(it);
		}
		else
		{
			++it;
		}
	}
}

std::string UsernameLookup::getCachedName(sf::Uint64 steamID) const
{
	auto it = cache.find(steamID);
	if (it == cache.end())
	{
		return "";
	}
	else
	{
		return it->second.name;
	}
}

std::string UsernameLookup::parseResponse(const std::string& response)
{
	static const std::string unknown = "[unknown]";

	cfg::JSONConfig config;
	try
	{
		config.loadFromString(response);
	}
	catch (std::exception & e)
	{
		debug() << e.what();
		return unknown;
	}

	std::string name = config.readValue("result[0].username").content;

	return name.empty() ? unknown : name;
}
