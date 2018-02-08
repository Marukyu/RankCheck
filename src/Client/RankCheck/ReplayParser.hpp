#ifndef SRC_CLIENT_RANKCHECK_REPLAYPARSER_HPP_
#define SRC_CLIENT_RANKCHECK_REPLAYPARSER_HPP_

#include <Client/RankCheck/PlayerData.hpp>
#include <SFML/Config.hpp>
#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

class DataStream;

class ReplayParser
{
public:
	ReplayParser(std::string replayFile);
	~ReplayParser();

	struct PlayerInfo
	{
		PlayerData player;
		PlayerData sponsor;
	};

	struct ReplayInfo
	{
		struct Hash
		{
			Hash() = default;
			Hash(std::string hash);

			std::array<sf::Uint32, 5> data;
		};

		bool countStats = false;
		PlayerData::Team localTeam = PlayerData::UnknownTeam;
		Hash hash;
		std::vector<PlayerInfo> players;
	};

	ReplayInfo parse();

private:
	std::string replayFile;
};

namespace std
{
template<> struct hash<ReplayParser::ReplayInfo::Hash>
{
	size_t operator()(const ReplayParser::ReplayInfo::Hash & hash) const
	{
		return size_t((1ull << 32) * hash.data[1] + hash.data[0]);
	}
};
}

bool operator==(const ReplayParser::ReplayInfo::Hash & hash1, const ReplayParser::ReplayInfo::Hash & hash2);
DataStream & operator<<(DataStream & stream, const ReplayParser::ReplayInfo::Hash & hash);
DataStream & operator>>(DataStream & stream, ReplayParser::ReplayInfo::Hash & hash);

#endif
