#ifndef SRC_CLIENT_RANKCHECK_GAMEFOLDER_HPP_
#define SRC_CLIENT_RANKCHECK_GAMEFOLDER_HPP_

#include <string>

namespace cfg
{
class Config;
}

class GameFolder
{
public:

	static GameFolder & getInstance();

	void initWithConfig(cfg::Config & config);

	std::string getFolder() const;

private:
	GameFolder();

	std::string gameFolder;
};

#endif
