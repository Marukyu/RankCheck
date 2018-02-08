#include <Client/RankCheck/NautsNames.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <map>

NautsNames& NautsNames::getInstance()
{
	static NautsNames instance;
	return instance;
}

void NautsNames::initWithConfig(const cfg::Config& config)
{
	static cfg::Int listSize("rankcheck.awesomenauts.classes.length");
	nauts.clear();
	for (cfg::Int::DataType i = 0; i < config.get(listSize); ++i)
	{
		Naut naut;
		naut.className = config.get(cfg::String("rankcheck.awesomenauts.classes[" + cNtoS(i) + "].className"));
		naut.name = config.get(cfg::String("rankcheck.awesomenauts.classes[" + cNtoS(i) + "].name"));
		nauts.push_back(naut);
	}
}

PlayerData::NautID NautsNames::resolveClassName(const std::string& className) const
{
	for (PlayerData::NautID i = 0; i < (PlayerData::NautID) nauts.size(); ++i)
	{
		if (nauts[i].className == className)
		{
			return i;
		}
	}
	return -1;
}

const std::string & NautsNames::getNautName(PlayerData::NautID id) const
{
	if (id < 0 || id >= (PlayerData::NautID) nauts.size())
	{
		static std::map<PlayerData::NautID, std::string> invalidNames;
		auto it = invalidNames.find(id);
		if (it != invalidNames.end())
		{
			return it->second;
		}
		else
		{
			std::string name = "Unknown (" + cNtoS(id) + ")";
			invalidNames[id] = name;
			return invalidNames[id];
		}
	}
	else
	{
		return nauts[id].name;
	}
}

const std::string & NautsNames::getNautClassName(PlayerData::NautID id) const
{
	if (id < 0 || id >= (PlayerData::NautID) nauts.size())
	{
		static std::map<PlayerData::NautID, std::string> invalidNames;
		auto it = invalidNames.find(id);
		if (it != invalidNames.end())
		{
			return it->second;
		}
		else
		{
			std::string name = cNtoS(id);
			invalidNames[id] = name;
			return invalidNames[id];
		}
	}
	else
	{
		return nauts[id].className;
	}
}

NautsNames::NautsNames()
{
}

