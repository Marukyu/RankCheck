#include <Client/RankCheck/GameFolder.hpp>
#include <Client/RankCheck/NetworkLogStartupReader.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/Hash.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <utility>

NetworkLogStartupReader::NetworkLogStartupReader()
{
}

NetworkLogStartupReader::~NetworkLogStartupReader()
{
}

void NetworkLogStartupReader::readNetworkLogs(cfg::Config & config)
{
	static cfg::List<cfg::Int> cfgKnownLogs("rankcheck.parsedNetlogs");

	knownLogs = config.get(cfgKnownLogs);

	std::string gameFolder = GameFolder::getInstance().getFolder();

	sf::Clock clock;

	for (std::size_t i = 1; i <= 5; ++i)
	{
		parseFile(gameFolder + "/ApplicationNetwork" + cNtoS(i) + ".log");
	}

	config.set(cfgKnownLogs, knownLogs);

	debug() << "Parsed network logs for IP mappings in " << clock.getElapsedTime().asSeconds() << "s";
}

const std::vector<NetworkLogStartupReader::IPMapping>& NetworkLogStartupReader::getIPMappings() const
{
	return ipMappings;
}

void NetworkLogStartupReader::parseFile(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file.good())
	{
		debug() << "Failed to open network log " << filename << " for IP parsing.";
		return;
	}

	debug() << "Parsing network log " << filename << "...";

	file.seekg(0, std::ios::end);
	std::size_t filesize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string line;

	if (std::getline(file, line))
	{
		sf::Int64 hash = dataHash32(line.data(), line.size());
		hash ^= sf::Int64(filesize);
		if (std::find(knownLogs.begin(), knownLogs.end(), hash) != knownLogs.end())
		{
			debug() << "Skipping previous parsed network log " << filename;
			return;
		}
		else
		{
			knownLogs.push_back(hash);

			while (knownLogs.size() > 5)
			{
				knownLogs.erase(knownLogs.begin());
			}
		}
	}
	else
	{
		debug() << "Failed to read network log " << filename << " for IP parsing.";
		return;
	}

	while (std::getline(file, line))
	{
		parseLine(line);
	}

	debug() << "Mapping IPs...";

	std::size_t count = mapIPs();
	debug() << "Found " << count << " SteamID/IP pairs.";

	nguidIpMap.clear();
	nguidConnectionMap.clear();
	connectionPlatformAccountMap.clear();
	platformAccountSteamIDMap.clear();
}

void NetworkLogStartupReader::parseLine(const std::string& line)
{
	std::size_t found = line.find_first_of('\t');
	if (found != std::string::npos)
	{
		found = line.find_first_of('\t', found + 1);
		if (found != std::string::npos)
		{
			std::string sub = line.substr(found + 1);
			parseRakNetIP(sub);
			parseNguidResolver(sub);
			parsePlatformAccountLinker(sub);
			parsePlatformAccountResolver(sub);
		}
	}
}

void NetworkLogStartupReader::parseRakNetIP(const std::string& line)
{
	static const std::string rakNetKeyword("RakNetWrapper1:  ");
	static const std::regex rakNetRegex("RakNetWrapper1:  Successfully connected to  "
		"nguid_([0-9]+)  ([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+).*");

	if (stringStartsWith(line, rakNetKeyword))
	{
		std::smatch result;
		if (std::regex_match(line, result, rakNetRegex) && result.size() == 3)
		{
			nguidIpMap[cStoUL(result[1])] = sf::IpAddress(result[2]);
		}
	}
}

void NetworkLogStartupReader::parseNguidResolver(const std::string& line)
{
	static const std::string nguidRakKeyword("nguid_");
	static const std::regex nguidRakRegex("nguid_([0-9]+)  =  "
		"nguid_([0-9]+)_r[0-9]+_s[0-9]+_rak[0-9]+\\([0-9]+\\)");

	if (stringStartsWith(line, nguidRakKeyword))
	{
		std::smatch result;
		if (std::regex_match(line, result, nguidRakRegex) && result.size() == 3)
		{
			nguidConnectionMap[cStoUL(result[1])] = cStoUL(result[2]);
		}
	}
}

void NetworkLogStartupReader::parsePlatformAccountLinker(const std::string& line)
{
	static const std::string nguidPaKeyword("nguid_");
	static const std::regex nguidPaRegex("nguid_[0-9]+  =  "
		"nguid_([0-9]+)_r[0-9]+_s[0-9]+_steam_pa_([0-9]+)\\([0-9]+\\)");

	if (stringStartsWith(line, nguidPaKeyword))
	{
		std::smatch result;
		if (std::regex_match(line, result, nguidPaRegex) && result.size() == 3)
		{
			connectionPlatformAccountMap[cStoUL(result[1])] = cStoUL(result[2]);
		}
	}
}

void NetworkLogStartupReader::parsePlatformAccountResolver(const std::string& line)
{
	static const std::string paSteamKeyword("pa_");
	static const std::regex paSteamRegex("pa_([0-9]+)  =  pa_.*#([0-9]+)");

	if (stringStartsWith(line, paSteamKeyword))
	{
		std::smatch result;
		if (std::regex_match(line, result, paSteamRegex) && result.size() == 3)
		{
			platformAccountSteamIDMap[cStoUL(result[1])] = cStoUL(result[2]);
		}
	}
}

std::size_t NetworkLogStartupReader::mapIPs()
{
	std::size_t count = 0;
	for (auto nguidIp : nguidIpMap)
	{
		auto nguidConnection = nguidConnectionMap.find(nguidIp.first);
		if (nguidConnection != nguidConnectionMap.end())
		{
			auto connectionPlatformAccount = connectionPlatformAccountMap.find(nguidConnection->second);
			if (connectionPlatformAccount != connectionPlatformAccountMap.end())
			{
				auto platformAccountSteamID = platformAccountSteamIDMap.find(connectionPlatformAccount->second);
				if (platformAccountSteamID != platformAccountSteamIDMap.end())
				{
					IPMapping mapping;
					mapping.steamID = platformAccountSteamID->second;
					mapping.ip = nguidIp.second;
					ipMappings.push_back(mapping);
					++count;
				}
			}
		}
	}
	return count;
}
