#ifndef SRC_CLIENT_RANKCHECK_LEAGUEREADER_HPP_
#define SRC_CLIENT_RANKCHECK_LEAGUEREADER_HPP_

#include <cstddef>
#include <string>
#include <vector>

namespace cfg
{
class Config;
}

class LeagueReader
{
public:

	static LeagueReader & getInstance();

	void initWithConfig(const cfg::Config & config);

	int getLeagueForRank(int rank) const;

	void parseLine(const std::string & line);

	void setEntryCount(std::size_t entryCount);

private:

	LeagueReader();

	std::vector<int> minRanks;
	bool complete;
};


#endif
