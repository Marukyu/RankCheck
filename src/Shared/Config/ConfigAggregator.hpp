#ifndef SRC_SHARED_CONFIG_CONFIGAGGREGATOR_HPP_
#define SRC_SHARED_CONFIG_CONFIGAGGREGATOR_HPP_

#include <Shared/Config/ConfigSource.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace cfg
{

class ConfigAggregator : public ConfigSource
{
public:
	ConfigAggregator();
	virtual ~ConfigAggregator();

	void addConfig(std::shared_ptr<ConfigSource> config, int order);
	void removeConfig(ConfigSource & config);
	bool hasConfig(ConfigSource & config) const;
	void clearConfigs();

	void setConfigOrder(ConfigSource & config, int order);
	int getConfigOrder(ConfigSource & config) const;

	void setWritableConfig(ConfigSource * config);
	ConfigSource * getWritableConfig() const;

	virtual Value readValue(std::string key) const override;
	virtual void writeValue(std::string key, Value value) override;

private:

	struct ConfigEntry
	{
		ConfigEntry() :
			config(nullptr),
			order(0)
		{
		}

		ConfigEntry(std::shared_ptr<ConfigSource> config, int order) :
			config(config),
			order(order)
		{
		}

		std::shared_ptr<ConfigSource> config;
		int order;
	};

	std::size_t findConfig(ConfigSource & config) const;
	void sort();

	std::vector<ConfigEntry> myConfigEntries;
	ConfigSource * myWritableConfig;
};

}

#endif
