#ifndef SRC_CLIENT_RANKCHECK_NETWORKLOGSTARTUPREADER_HPP_
#define SRC_CLIENT_RANKCHECK_NETWORKLOGSTARTUPREADER_HPP_

#include <SFML/Config.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace cfg
{
class Config;
}

/**
 * Parses IP/SteamID mappings from all "ApplicationNetwork[1-5].log" files for storage in the local player database.
 *
 * Only files that have not been parsed yet are considered.
 */
class NetworkLogStartupReader
{
public:

	struct IPMapping
	{
		sf::Uint64 steamID;
		sf::IpAddress ip;
	};

	NetworkLogStartupReader();
	virtual ~NetworkLogStartupReader();

	void readNetworkLogs(cfg::Config & config);

	const std::vector<IPMapping> & getIPMappings() const;

private:

	void parseFile(const std::string & filename);
	void parseLine(const std::string & line);

	void parseRakNetIP(const std::string & line);
	void parseNguidResolver(const std::string & line);
	void parsePlatformAccountLinker(const std::string & line);
	void parsePlatformAccountResolver(const std::string & line);

	std::size_t mapIPs();

	std::unordered_map<sf::Uint64, sf::IpAddress> nguidIpMap;
	std::unordered_map<sf::Uint64, sf::Uint64> nguidConnectionMap;
	std::unordered_map<sf::Uint64, sf::Uint64> connectionPlatformAccountMap;
	std::unordered_map<sf::Uint64, sf::Uint64> platformAccountSteamIDMap;

	std::vector<IPMapping> ipMappings;

	std::vector<sf::Int64> knownLogs;
};

#endif
