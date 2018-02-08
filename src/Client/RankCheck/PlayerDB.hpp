#ifndef SRC_CLIENT_RANKCHECK_PLAYERDB_HPP_
#define SRC_CLIENT_RANKCHECK_PLAYERDB_HPP_

#include <Client/RankCheck/PlayerData.hpp>
#include <Client/RankCheck/ReplayParser.hpp>
#include <SFML/Config.hpp>
#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

class DataStream;

class PlayerDB
{
public:

	struct EntryV0
	{
		std::map<std::string, sf::Uint32> names;

		sf::Uint64 sponsorSteamID = 0;
		sf::Uint32 allyCount = 0;
		sf::Uint32 enemyCount = 0;

		std::string getCommonName() const;
		void addName(const std::string & name);
	};

	struct EntryV1 : public EntryV0
	{
		EntryV1() = default;
		EntryV1(const EntryV0 & v0) :
			EntryV0(v0),
			lastKnownLeague(-1),
			lastKnownRank(-1),
			lastKnownRating(-1)
		{
		}

		sf::Int8 lastKnownLeague;
		sf::Int32 lastKnownRank;
		sf::Int32 lastKnownRating;
	};

	struct EntryV2 : public EntryV0
	{
		EntryV2() = default;
		EntryV2(const EntryV0 & v0) :
			EntryV0(v0)
		{
		}

		std::vector<sf::IpAddress> ipAddresses;

		sf::IpAddress getRecentIP() const;
		void addIP(sf::IpAddress ip);

		void clear();
	};

	PlayerDB();
	~PlayerDB();

	void clear();

	void load();
	void save() const;

	void addPlayerToStats(const PlayerData & data, PlayerData::Team localTeam, sf::Uint64 sponsorSteamID);
	void fillPlayerData(PlayerData & data, std::size_t recursion = 0) const;
	void fillPlayerData(PlayerData & data, PlayerData & sponsor, std::size_t recursion = 0) const;
	void addIPMapping(sf::Uint64 steamID, sf::IpAddress ip);
	EntryV2 getEntry(sf::Uint64 steamID) const;

	bool hasReplayHash(ReplayParser::ReplayInfo::Hash hash) const;
	void addReplayHash(ReplayParser::ReplayInfo::Hash hash);

	std::size_t getReplayHashCount() const;
	std::size_t getPlayerCount() const;

	bool empty() const;

	const std::map<sf::Uint64, EntryV2> & getEntries() const;

private:

	std::map<sf::Uint64, EntryV2> entries;
	std::unordered_set<ReplayParser::ReplayInfo::Hash> replayHashes;
};

DataStream & operator<<(DataStream & stream, const PlayerDB::EntryV0 & entry);
DataStream & operator>>(DataStream & stream, PlayerDB::EntryV0 & entry);
DataStream & operator<<(DataStream & stream, const PlayerDB::EntryV1 & entry);
DataStream & operator>>(DataStream & stream, PlayerDB::EntryV1 & entry);
DataStream & operator<<(DataStream & stream, const PlayerDB::EntryV2 & entry);
DataStream & operator>>(DataStream & stream, PlayerDB::EntryV2 & entry);

#endif
