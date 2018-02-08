#ifndef SRC_CLIENT_RANKCHECK_PLAYERDATA_HPP_
#define SRC_CLIENT_RANKCHECK_PLAYERDATA_HPP_

#include <SFML/Config.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <string>

struct PlayerData
{
	using NautID = int;

	enum Team
	{
		UnknownTeam,
		Red,
		Blue
	};

	enum Type
	{
		Player,
		SponsoredPlayer,
		Sponsor
	};

	sf::Uint64 steamID = 0;

	sf::IpAddress ip = sf::IpAddress::None;
	std::string country = "";
	std::string countryCode = "";

	bool hasLeaderboardData = false;
	bool downloadFailed = false;
	bool notFound = false;

	bool isLocal = false;
	Type type = Player;

	std::string currentName = "";
	std::string commonName = "";

	Team team = UnknownTeam;
	NautID currentNaut = 0;
	int currentSkin = 0;

	NautID mainNaut = 0;

	int league = 0;
	int rank = 0;
	int rating = 0;

	int prevLeague = 0;
	int prevRank = 0;
	int prevRating = 0;

	int winCount = 0;
	int lossCount = 0;

	int winCountTotal = 0;
	int lossCountTotal = 0;

	int prevAllyCount = 0;
	int prevEnemyCount = 0;

	std::string format(const std::string & input) const;
	bool evaluate(const std::string & input) const;
};

#endif
