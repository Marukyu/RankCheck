#include <Client/RankCheck/LeagueReader.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>

namespace cfg
{
class Config;
}

LeagueReader& LeagueReader::getInstance()
{
	static LeagueReader singleton;
	return singleton;
}

void LeagueReader::initWithConfig(const cfg::Config& config)
{
}

int LeagueReader::getLeagueForRank(int rank) const
{
	if (rank < 1)
	{
		return 0;
	}

	int league;
	for (league = 0; league < (int)minRanks.size(); ++league)
	{
		if (rank <= minRanks[league])
		{
			break;
		}
	}
	return league + 1;
}

void LeagueReader::parseLine(const std::string& line)
{
	if (complete)
	{
		return;
	}

	static std::string keyDivision = "Division  ";

	std::size_t found = line.find(keyDivision);
	if (found != std::string::npos)
	{
		std::vector<std::string> div;
		splitString(line.substr(found + keyDivision.size()), " ", div, true);
		std::size_t divIndex = cStoUI(div[0]);
		if (divIndex < minRanks.size() && div[1] == "minRank")
		{
			std::size_t minRank = cStoUI(div[2]);
			minRanks[divIndex] = minRank;

			complete = true;
			for (int rank : minRanks)
			{
				if (rank < 0)
				{
					complete = false;
					break;
				}
			}
		}
	}
}

void LeagueReader::setEntryCount(std::size_t entryCount)
{
	static const std::vector<float> factors = {
		0.015,
		0.03,
		0.08,
		0.13,
		0.235,
		0.34,
		0.474,
		0.6785
	};

	minRanks.clear();
	for (auto factor : factors)
	{
		minRanks.push_back(factor * entryCount);
	}
	// Fix L1 size
	if (!minRanks.empty() && minRanks[0] > 250)
	{
		minRanks[0] = 250;
	}

	complete = true;
}

LeagueReader::LeagueReader()
{
	complete = false;
	minRanks.resize(9);
	for (int & rank : minRanks)
	{
		rank = -1;
	}
}

