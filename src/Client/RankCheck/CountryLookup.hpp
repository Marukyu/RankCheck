#ifndef SRC_CLIENT_RANKCHECK_COUNTRYLOOKUP_HPP_
#define SRC_CLIENT_RANKCHECK_COUNTRYLOOKUP_HPP_

#include <SFML/Network/IpAddress.hpp>
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

class CountryLookup
{
public:

	static const std::string invalidCountry;

	CountryLookup();
	~CountryLookup();

	using Callback = std::function<void(std::string)>;

	void setHost(std::string host, unsigned short port);
	void setUriParameters(std::string prefix, std::string suffix);

	void lookup(sf::IpAddress address, Callback callback);
	void process();

private:

	struct Connection
	{
		sf::IpAddress address;
		std::atomic_bool done = ATOMIC_VAR_INIT(false);
		std::thread thread;
		std::string result;
		Callback callback;
	};

	std::map<sf::IpAddress, std::string> cache;
	std::vector<std::unique_ptr<Connection> > connections;
	std::string host;
	std::string uriPrefix;
	std::string uriSuffix;
	unsigned short port;
};

#endif
