#include <Client/RankCheck/PlayerDB.hpp>
#include <Poco/Path.h>
#include <Shared/Utils/DataStream.hpp>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <cstddef>
#include <utility>

std::string PlayerDB::EntryV0::getCommonName() const
{
	std::string commonName;
	sf::Uint32 commonCount = 0;
	for (const auto & name : names)
	{
		if (commonCount < name.second)
		{
			commonName = name.first;
			commonCount = name.second;
		}
	}
	return commonName;
}

void PlayerDB::EntryV0::addName(const std::string& name)
{
	if (name == "[unknown]" || name.empty())
	{
		return;
	}

	if (names.count(name) == 0)
	{
		names[name] = 1;
	}
	else
	{
		names[name]++;
	}
}

PlayerDB::PlayerDB()
{
	Poco::Path dir(Poco::Path::cacheHome());
	dir.pushDirectory("rankcheck");
	DB_FILENAME = Poco::Path(dir, "players.db").toString();
}

PlayerDB::~PlayerDB()
{
}

static constexpr sf::Int32 HEADER = 1333337;

void PlayerDB::load()
{
	clear();

	sf::Int32 header;
	sf::Int16 version;

	DataStream stream;
	if (!stream.openInFile(DB_FILENAME))
	{
		// No DB? Just treat it as empty.
		return;
	}

	stream >> header >> version;

	if (stream.isValid() && header == HEADER)
	{
		std::map<sf::Uint64, EntryV0> entriesv0;

		switch (version)
		{
		case 0:
			stream >> entriesv0;
			break;
		case 1:
			stream >> entriesv0 >> replayHashes;
			break;
		case 2:
			stream >> entries >> replayHashes;
			break;
		default:
			throw Error("Error loading player database: unrecognized version " + cNtoS(version));
		}

		for (const auto & entry : entriesv0)
		{
			entries.emplace(entry.first, entry.second);
		}
	}
	else
	{
		throw Error("Error loading player database: corrupt header");
	}
}

void PlayerDB::save() const
{
	sf::Int32 header = HEADER;
	sf::Int16 version = 2;

	DataStream stream;
	if (!stream.openOutFile(DB_FILENAME))
	{
		throw Error("Error saving player database: could not write " + DB_FILENAME);
	}

	stream << header << version << entries << replayHashes;
}

void PlayerDB::addPlayerToStats(const PlayerData& data, PlayerData::Team localTeam, sf::Uint64 sponsorSteamID)
{
	//bool isNew = (entries.count(data.steamID) == 0);

	auto & entry = entries[data.steamID];

	entry.addName(data.currentName);

	if (localTeam != PlayerData::UnknownTeam)
	{
		if (data.team == localTeam)
		{
			entry.allyCount++;
		}
		else
		{
			entry.enemyCount++;
		}
	}

	if (sponsorSteamID != 0)
	{
		entry.sponsorSteamID = sponsorSteamID;
	}
}

void PlayerDB::fillPlayerData(PlayerData& data, std::size_t recursion) const
{
	PlayerData dummy;
	fillPlayerData(data, dummy, recursion);
}

void PlayerDB::fillPlayerData(PlayerData & data, PlayerData & sponsor, std::size_t recursion) const
{
	auto it = entries.find(data.steamID);

	if (it == entries.end() || recursion > 50)
	{
		return;
	}

	const auto & entry = it->second;

	auto commonName = entry.getCommonName();
	if (data.currentName != commonName)
	{
		data.commonName = commonName;
	}

	data.prevAllyCount = entry.allyCount;
	data.prevEnemyCount = entry.enemyCount;

	if (!data.isLocal && !entry.ipAddresses.empty())
	{
		data.ip = entry.ipAddresses.back();
	}

	/*
	if (entry.sponsorSteamID != 0)
	{
		data.type = PlayerData::SponsoredPlayer;
		sponsor.steamID = entry.sponsorSteamID;
		sponsor.type = PlayerData::Sponsor;
		fillPlayerData(sponsor, recursion + 1);
	}
	*/
}

PlayerDB::EntryV2 PlayerDB::getEntry(sf::Uint64 steamID) const
{
	auto it = entries.find(steamID);

	if (it == entries.end())
	{
		static PlayerDB::EntryV2 ded;
		return ded;
	}
	else
	{
		return it->second;
	}
}

bool PlayerDB::hasReplayHash(ReplayParser::ReplayInfo::Hash hash) const
{
	return replayHashes.count(hash);
}

void PlayerDB::addReplayHash(ReplayParser::ReplayInfo::Hash hash)
{
	replayHashes.insert(hash);
}

std::size_t PlayerDB::getReplayHashCount() const
{
	return replayHashes.size();
}

std::size_t PlayerDB::getPlayerCount() const
{
	return entries.size();
}

void PlayerDB::clear()
{
	for (auto & entry : entries)
	{
		entry.second.clear();
	}

	replayHashes.clear();
}

void PlayerDB::addIPMapping(sf::Uint64 steamID, sf::IpAddress ip)
{
	entries[steamID].addIP(ip);
}

bool PlayerDB::empty() const
{
	return entries.empty();
}

DataStream& operator <<(DataStream& stream, const sf::IpAddress & ip)
{
	return stream << ip.toInteger();
}

DataStream& operator >>(DataStream& stream, sf::IpAddress & ip)
{
	sf::Uint32 value;
	stream >> value;
	ip = sf::IpAddress(value);
	return stream;
}

DataStream& operator <<(DataStream& stream, const PlayerDB::EntryV0& entry)
{
	return stream << entry.names << entry.allyCount << entry.enemyCount << entry.sponsorSteamID;
}

DataStream& operator >>(DataStream& stream, PlayerDB::EntryV0& entry)
{
	return stream >> entry.names >> entry.allyCount >> entry.enemyCount >> entry.sponsorSteamID;
}

DataStream& operator <<(DataStream& stream, const PlayerDB::EntryV1& entry)
{
	return stream << entry.names << entry.allyCount << entry.enemyCount << entry.sponsorSteamID << entry.lastKnownLeague
		<< entry.lastKnownRank << entry.lastKnownRating;
}

DataStream& operator >>(DataStream& stream, PlayerDB::EntryV1& entry)
{
	return stream >> entry.names >> entry.allyCount >> entry.enemyCount >> entry.sponsorSteamID >> entry.lastKnownLeague
		>> entry.lastKnownRank >> entry.lastKnownRating;
}

DataStream& operator <<(DataStream& stream, const PlayerDB::EntryV2& entry)
{
	return stream << entry.names << entry.allyCount << entry.enemyCount << entry.sponsorSteamID << entry.ipAddresses;
}

DataStream& operator >>(DataStream& stream, PlayerDB::EntryV2& entry)
{
	return stream >> entry.names >> entry.allyCount >> entry.enemyCount >> entry.sponsorSteamID >> entry.ipAddresses;
}

sf::IpAddress PlayerDB::EntryV2::getRecentIP() const
{
	return ipAddresses.empty() ? sf::IpAddress::None : ipAddresses.back();
}

void PlayerDB::EntryV2::addIP(sf::IpAddress ip)
{
	static const std::size_t MAX_IP_COUNT = 10;

	for (auto it = ipAddresses.begin(); it != ipAddresses.end(); ++it)
	{
		if (ip == *it)
		{
			ipAddresses.erase(it);
			break;
		}
	}
	ipAddresses.push_back(ip);

	while (ipAddresses.size() > MAX_IP_COUNT)
	{
		ipAddresses.erase(ipAddresses.begin());
	}
}

void PlayerDB::EntryV2::clear()
{
	names.clear();
	sponsorSteamID = 0;
	allyCount = 0;
	enemyCount = 0;
	// Preserve IPs
}

const std::map<sf::Uint64, PlayerDB::EntryV2>& PlayerDB::getEntries() const
{
	return entries;
}
