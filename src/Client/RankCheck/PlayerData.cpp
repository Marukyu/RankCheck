#include <Client/RankCheck/CountryLookup.hpp>
#include <Client/RankCheck/NautsNames.hpp>
#include <Client/RankCheck/PlayerData.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <utility>

static std::map<std::string, std::function<std::string(const PlayerData &)>> expressions =
{
	{
		"always", [](const PlayerData & data)
		{
			return "1";
		}
	},
	{
		"islocal", [](const PlayerData & data)
		{
			return data.isLocal ? "1" : "";
		}
	},
	{
		"hasleaderboarddata", [](const PlayerData & data)
		{
			return data.hasLeaderboardData ? "1" : "";
		}
	},
	{
		"steamid", [](const PlayerData & data)
		{
			return cNtoS(data.steamID);
		}
	},
	{
		"ip", [](const PlayerData & data)
		{
			return data.ip == sf::IpAddress::None ? "" : data.ip.toString();
		}
	},
	{
		"country", [](const PlayerData & data)
		{
			return data.country;
		}
	},
	{
		"countrycode", [](const PlayerData & data)
		{
			return data.countryCode;
		}
	},
	{
		"hascountry", [](const PlayerData & data)
		{
			return (data.countryCode != CountryLookup::invalidCountry && !data.countryCode.empty()) ? "1" : "";
		}
	},
	{
		"accounttype", [](const PlayerData & data)
		{
			switch (data.type)
			{
			case PlayerData::Player:
			default:
				return "";
			case PlayerData::SponsoredPlayer:
				return "Alt";
			case PlayerData::Sponsor:
				return "Main";
			}
		}
	},
	{
		"name", [](const PlayerData & data)
		{
			return data.currentName.empty() ? data.commonName : data.currentName;
		}
	},
	{
		"commonname", [](const PlayerData & data) -> std::string
		{
			if (data.currentName.empty())
			{
				return "";
			}
			else
			{
				return data.currentName == data.commonName ? "" : data.commonName;
			}
		}
	},
	{
		"team", [](const PlayerData & data)
		{
			switch (data.team)
			{
			default:
				return "";
			case PlayerData::Red:
				return "Red";
			case PlayerData::Blue:
				return "Blue";
			}
		}
	},
	{
		"currentnaut", [](const PlayerData & data)
		{
			return NautsNames::getInstance().getNautName(data.currentNaut);
		}
	},
	{
		"currentskin", [](const PlayerData & data)
		{
			return cNtoS(data.currentSkin);
		}
	},
	{
		"mainnaut", [](const PlayerData & data)
		{
			return NautsNames::getInstance().getNautName(data.mainNaut);
		}
	},
	{
		"rank", [](const PlayerData & data)
		{
			return cNtoS(data.rank);
		}
	},
	{
		"#rank", [](const PlayerData & data)
		{
			return data.rank == 0 ? "Unranked" : "#" + cNtoS(data.rank);
		}
	},
	{
		"rating", [](const PlayerData & data)
		{
			return cNtoS(data.rating);
		}
	},
	{
		"prevrank", [](const PlayerData & data)
		{
			return cNtoS(data.prevRank);
		}
	},
	{
		"prevrating", [](const PlayerData & data)
		{
			return cNtoS(data.prevRating);
		}
	},
	{
		"wincount", [](const PlayerData & data)
		{
			return cNtoS(data.winCount);
		}
	},
	{
		"losscount", [](const PlayerData & data)
		{
			return cNtoS(data.lossCount);
		}
	},
	{
		"matchcount", [](const PlayerData & data) -> std::string
		{
			return cNtoS(data.winCount + data.lossCount);
		}
	},
	{
		"matchcounttotal", [](const PlayerData & data) -> std::string
		{
			return cNtoS(data.winCountTotal + data.lossCountTotal);
		}
	},
	{
		"winpercent", [](const PlayerData & data) -> std::string
		{
			int totalMatches = data.winCount + data.lossCount;
			if (totalMatches == 0)
			{
				return "";
			}
			return cNtoS(0.01f * std::round(10000.f * float(data.winCount) / float(totalMatches)));
		}
	},
	{
		"winpercent%", [](const PlayerData & data) -> std::string
		{
			int totalMatches = data.winCount + data.lossCount;
			if (totalMatches == 0)
			{
				return "N/A";
			}
			return cNtoS(0.01f * std::round(10000.f * float(data.winCount) / float(totalMatches))) + " %";
		}
	},
	{
		"allycount", [](const PlayerData & data)
		{
			return cNtoS(data.prevAllyCount);
		}
	},
	{
		"enemycount", [](const PlayerData & data)
		{
			return cNtoS(data.prevEnemyCount);
		}
	},
};

std::string PlayerData::format(const std::string & input) const
{
	std::string output;
	std::string token;
	bool readingToken = false;

	for (std::size_t i = 0; i < input.size(); ++i)
	{
		if (readingToken)
		{
			if (input[i] == ']')
			{
				bool plural = false;
				if (!token.empty() && token[0] == '$')
				{
					plural = true;
				}

				auto expr = expressions.find(plural ? token.substr(1) : token);
				if (expr == expressions.end())
				{
					output += '[';
					output += token;
					output += "???]";
				}
				else
				{
					if (plural)
					{
						if (expr->second(*this) != "1")
						{
							output += "s";
						}
					}
					else
					{
						output += expr->second(*this);
					}
				}

				readingToken = false;
				token.clear();
			}
			else
			{
				token += ::tolower(input[i]);
			}
		}
		else
		{
			if (input[i] == '[')
			{
				readingToken = true;
			}
			else
			{
				output += input[i];
			}
		}
	}

	return output;
}

bool PlayerData::evaluate(const std::string & input) const
{
	if (input.empty())
	{
		return true;
	}

	auto tolowerString = [](std::string in)
	{
		std::transform(in.begin(), in.end(), in.begin(), ::tolower);
		return in;
	};

	bool negate = (input[0] == '!');
	bool expressionResult = false;

	auto expr = expressions.find(tolowerString(negate ? input.substr(1) : input));
	if (expr != expressions.end())
	{
		expressionResult = !expr->second(*this).empty();
	}

	return expressionResult != negate;
}
