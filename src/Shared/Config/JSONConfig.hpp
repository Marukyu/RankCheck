#ifndef SRC_SHARED_CONFIG_JSONCONFIGSOURCE_HPP_
#define SRC_SHARED_CONFIG_JSONCONFIGSOURCE_HPP_

#include <Shared/Config/ConfigSource.hpp>
#include <rapidjson/document.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class DataStream;

namespace cfg
{

class JSONConfig : public ConfigSource
{
public:

	static const std::string LENGTH_NODE;
	static const std::string LENGTH_SUFFIX;

	JSONConfig();
	virtual ~JSONConfig();

	void loadFromMemory(const char * data, std::size_t size);
	void loadFromString(const std::string & json);

	enum class Style
	{
		Pretty,
		Compact
	};

	std::string saveToString(Style style = Style::Pretty) const;

	virtual Value readValue(std::string key) const override;
	virtual void writeValue(std::string key, Value value) override;

private:

	struct PathComponent
	{
	public:

		PathComponent(std::string name) :
			name(std::move(name)),
			arrayIndex(0)
		{
		}

		PathComponent(unsigned int arrayIndex) :
			name("["),
			arrayIndex(arrayIndex)
		{
		}

		bool isArrayIndex() const
		{
			return !name.empty() && name[0] == '[';
		}

		unsigned int getArrayIndex() const
		{
			return arrayIndex;
		}

		const std::string & getName() const
		{
			return name;
		}

	private:
		std::string name;
		unsigned int arrayIndex;
	};

	std::vector<PathComponent> getPathComponents(const std::string & key) const;
	void checkPathString(const std::string & pathString) const;

	const rapidjson::Value * findKey(const std::string & key) const;
	rapidjson::Value & createOrFindKey(const std::string & key);

	bool isArrayLengthKey(const std::string & key) const;
	bool tryGetArrayLength(const std::string & key, std::size_t & arrayLength) const;
	bool trySetArrayLength(const std::string & key, std::size_t arrayLength);

	void setNodeValue(rapidjson::Value & node, Value value);
	Value getNodeValue(const rapidjson::Value & node) const;

	std::unique_ptr<rapidjson::Document> document;

	friend class JSONHandler;
};

}

#endif /* SRC_SHARED_CONFIG_JSONCONFIGSOURCE_HPP_ */
