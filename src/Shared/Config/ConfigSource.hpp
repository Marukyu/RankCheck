#ifndef SRC_SHARED_CONFIG_CONFIGSOURCE_HPP_
#define SRC_SHARED_CONFIG_CONFIGSOURCE_HPP_

#include <string>

namespace cfg
{
struct Value;

class ConfigSource
{
public:

	ConfigSource();
	virtual ~ConfigSource();

	virtual Value readValue(std::string key) const = 0;
	virtual void writeValue(std::string key, Value value) = 0;
};

}

#endif /* SRC_SHARED_CONFIG_CONFIGSOURCE_HPP_ */
