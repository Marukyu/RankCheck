#include <Shared/Config/ConfigAggregator.hpp>
#include <Shared/Config/DataTypes.hpp>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>

namespace cfg
{

ConfigAggregator::ConfigAggregator()
{
}

ConfigAggregator::~ConfigAggregator()
{
}

void ConfigAggregator::addConfig(std::shared_ptr<ConfigSource> config, int order)
{
	if (config != nullptr)
	{
		std::size_t index = findConfig(*config);

		if (index == myConfigEntries.size())
		{
			myConfigEntries.emplace_back(config, order);
			sort();
		}
	}
}

void ConfigAggregator::removeConfig(ConfigSource& config)
{
	std::size_t index = findConfig(config);

	if (index != myConfigEntries.size())
	{
		if (&config == myWritableConfig)
		{
			myWritableConfig = nullptr;
		}

		myConfigEntries.erase(myConfigEntries.begin() + index);
	}
}

bool ConfigAggregator::hasConfig(ConfigSource& config) const
{
	return findConfig(config) != myConfigEntries.size();
}

void ConfigAggregator::clearConfigs()
{
	myConfigEntries.clear();
	myWritableConfig = nullptr;
}

void ConfigAggregator::setConfigOrder(ConfigSource& config, int order)
{
	std::size_t index = findConfig(config);

	if (index != myConfigEntries.size())
	{
		myConfigEntries[index].order = order;
		sort();
	}
}

int ConfigAggregator::getConfigOrder(ConfigSource& config) const
{
	std::size_t index = findConfig(config);

	if (index != myConfigEntries.size())
	{
		return myConfigEntries[index].order;
	}

	return 0;
}

void ConfigAggregator::setWritableConfig(ConfigSource* config)
{
	if (config == nullptr || hasConfig(*config))
	{
		myWritableConfig = config;
	}
}

ConfigSource* ConfigAggregator::getWritableConfig() const
{
	return myWritableConfig;
}

Value ConfigAggregator::readValue(std::string key) const
{
	for (const auto & entry : myConfigEntries)
	{
		Value value = entry.config->readValue(key);
		if (value.type != Value::Type::Missing)
		{
			return std::move(value);
		}
	}

	return Value();
}

void ConfigAggregator::writeValue(std::string key, Value value)
{
	if (myWritableConfig != nullptr)
	{
		myWritableConfig->writeValue(key, value);
	}
}

std::size_t ConfigAggregator::findConfig(ConfigSource& config) const
{
	for (std::size_t i = 0; i < myConfigEntries.size(); ++i)
	{
		if (myConfigEntries[i].config.get() == &config)
		{
			return i;
		}
	}
	return myConfigEntries.size();
}

void ConfigAggregator::sort()
{
	std::stable_sort(myConfigEntries.begin(), myConfigEntries.end(), [](const ConfigEntry & a, const ConfigEntry & b)
	{
		return a.order < b.order;
	});
}

}
