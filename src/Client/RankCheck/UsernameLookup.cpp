#include <Client/RankCheck/UsernameLookup.hpp>
#include <SFML/Network/Http.hpp>
#include <Shared/External/PugiXML/pugixml.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <algorithm>
#include <iostream>
#include <iterator>

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

	conn->thread = std::thread([connPtr]()
	{
		sf::Http http;
		http.setHost("http://steamcommunity.com");

		sf::Http::Request request;
		request.setUri("profiles/" + cNtoS(connPtr->steamID) + "?xml=1");

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

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_buffer(response.data(), response.size());

	if (!result)
	{
		return unknown;
	}

	return doc.root().child("profile").child("steamID").text().as_string(unknown.c_str());
}
