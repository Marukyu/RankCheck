#ifndef SRC_CLIENT_RANKCHECK_NAUTSNAMES_HPP_
#define SRC_CLIENT_RANKCHECK_NAUTSNAMES_HPP_

#include <Client/RankCheck/PlayerData.hpp>
#include <string>
#include <vector>

namespace cfg
{
class Config;
}

class NautsNames
{
public:

	static NautsNames & getInstance();

	void initWithConfig(const cfg::Config & config);

	PlayerData::NautID resolveClassName(const std::string & className) const;
	const std::string & getNautName(PlayerData::NautID id) const;
	const std::string & getNautClassName(PlayerData::NautID id) const;

private:

	NautsNames();

	struct Naut
	{
		std::string name;
		std::string className;
	};
	std::vector<Naut> nauts;
};

#endif
