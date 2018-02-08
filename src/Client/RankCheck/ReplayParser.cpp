#include <Client/RankCheck/NautsNames.hpp>
#include <Client/RankCheck/ReplayParser.hpp>
#include <Shared/External/PugiXML/pugixml.hpp>
#include <Shared/Utils/DataStream.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <map>
#include <utility>

ReplayParser::ReplayParser(std::string replayFile) :
	replayFile(replayFile)
{
}

ReplayParser::~ReplayParser()
{
}

static const std::pair<std::string, sf::Uint64> parseSteamID(const std::string & steamIDString)
{
	std::pair<std::string, sf::Uint64> ret;
	std::size_t hashPos = steamIDString.find_last_of('#');
	if (hashPos == std::string::npos)
	{
		return
		{	"", 0};
	}
	else
	{
		return
		{	steamIDString.substr(0, hashPos), cStoN<sf::Uint64>(steamIDString.substr(hashPos + 1))};
	}
}

ReplayParser::ReplayInfo ReplayParser::parse()
{
	std::map<sf::Uint64, std::size_t> playerMap;
	ReplayInfo info;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(replayFile.c_str());

	if (!result)
	{
		debug() << "Failed to parse " << replayFile << ": " << result.description();
		return info;
	}

	pugi::xml_node replay = doc.root().child("Replay");

	if (replay.attribute("onlineMatch").as_uint(1))
	{
		info.countStats = true;
	}

	info.hash = ReplayInfo::Hash(replay.child("Hash").attribute("hash").as_string(""));

	pugi::xml_node characters = replay.child("Characters");

	for (const auto & character : characters)
	{
		if (character.attribute("isBot").as_uint(1) == 1)
		{
			continue;
		}

		PlayerData player;

		auto steamID = parseSteamID(character.attribute("steamId").as_string("#0"));
		auto sponsorID = parseSteamID(
			character.attribute("sponsorSteamId").as_string(character.attribute("steamId").as_string("#0")));

		player.steamID = steamID.second;
		player.currentName = steamID.first;
		//player.currentName = character.attribute("name").as_string();
		player.team = character.attribute("team").as_uint(0) == 0 ? PlayerData::Red : PlayerData::Blue;
		player.currentNaut = NautsNames::getInstance().resolveClassName(character.attribute("className").as_string());
		player.currentSkin = character.attribute("skin").as_int(0);
		player.isLocal = character.attribute("local").as_uint(0);

		if (player.isLocal)
		{
			info.localTeam = player.team;
		}

		PlayerData sponsor;

		if (sponsorID != steamID && sponsorID.second != 0)
		{
			sponsor = player;
			sponsor.type = PlayerData::Sponsor;
			sponsor.currentName = sponsorID.first;
			sponsor.steamID = sponsorID.second;
			sponsor.currentNaut = 0;
			sponsor.currentSkin = 0;
			sponsor.isLocal = false;

			player.type = PlayerData::SponsoredPlayer;
		}
		else
		{
			player.type = PlayerData::Player;
		}

		PlayerInfo playerInfo { player, sponsor };

		if (playerMap.count(steamID.second))
		{
			info.players[playerMap[steamID.second]] = playerInfo;
		}
		else
		{
			playerMap[steamID.second] = info.players.size();
			info.players.push_back(playerInfo);
		}
	}

	return info;
}

ReplayParser::ReplayInfo::Hash::Hash(std::string hash)
{
	static const std::size_t groupSize = 8;
	static const std::size_t groupCount = 5;

	if (hash.size() < groupCount * groupSize)
	{
		hash = std::string('0', groupCount * groupSize - hash.size()) + hash;
	}

	for (std::size_t i = 0; i < groupCount; ++i)
	{
		data[i] = 0;
		for (std::size_t j = 0; j < groupSize; ++j)
		{
			data[i] <<= 4;
			data[i] |= hex2Num(hash[i * groupSize + j]);
		}
	}
}

DataStream & operator<<(DataStream & stream, const ReplayParser::ReplayInfo::Hash & hash)
{
	for (auto data : hash.data)
	{
		stream << data;
	}
	return stream;
}

DataStream & operator>>(DataStream & stream, ReplayParser::ReplayInfo::Hash & hash)
{
	for (auto & data : hash.data)
	{
		stream >> data;
	}
	return stream;
}

bool operator ==(const ReplayParser::ReplayInfo::Hash& hash1, const ReplayParser::ReplayInfo::Hash& hash2)
{
	for (std::size_t i = 0; i < hash1.data.size(); ++i)
	{
		if (hash1.data[i] != hash2.data[i])
		{
			return false;
		}
	}
	return true;
}
