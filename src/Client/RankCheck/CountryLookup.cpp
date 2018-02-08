#include <Client/RankCheck/CountryLookup.hpp>
#include <SFML/Network/Http.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <utility>

const std::string CountryLookup::invalidCountry = "XX";

CountryLookup::CountryLookup() :
	port(0)
{
	cache.emplace(sf::IpAddress::None, invalidCountry);
}

CountryLookup::~CountryLookup()
{
	for (const auto & conn : connections)
	{
		conn->thread.join();
	}
}

void CountryLookup::lookup(sf::IpAddress address, Callback callback)
{
	auto cacheEntry = cache.find(address);
	if (cacheEntry != cache.end())
	{
		callback(cacheEntry->second);
		return;
	}

	std::unique_ptr<Connection> conn = makeUnique<Connection>();
	conn->callback = callback;
	conn->address = address;
	Connection * connPtr = conn.get();

	conn->thread = std::thread([this,connPtr]()
	{
		sf::Http http;
		http.setHost(host, port);

		std::string addressString;
		if (connPtr->address != sf::IpAddress::LocalHost)
		{
			addressString = connPtr->address.toString();
		}

		sf::Http::Request request;
		request.setUri(uriPrefix + addressString + uriSuffix);

		sf::Http::Response response = http.sendRequest(request);

		std::vector<std::string> split;
		splitString(response.getBody(), ",", split);
		connPtr->result = (split.size() < 2 || split[1].size() != 2) ? invalidCountry : split[1];
		connPtr->done = true;
	});

	connections.push_back(std::move(conn));
}

void CountryLookup::setHost(std::string host, unsigned short port)
{
	this->host = host;
	this->port = port;
}

void CountryLookup::setUriParameters(std::string prefix, std::string suffix)
{
	uriPrefix = prefix;
	uriSuffix = suffix;
}

void CountryLookup::process()
{
	for (auto it = connections.begin(); it != connections.end();)
	{
		Connection & connection = **it;
		if (connection.done)
		{
			cache.insert(std::make_pair(connection.address, connection.result));
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
