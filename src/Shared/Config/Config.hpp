#ifndef SRC_SHARED_CONFIG_CONFIGCACHE_HPP_
#define SRC_SHARED_CONFIG_CONFIGCACHE_HPP_

#include <Shared/Config/ConfigSource.hpp>
#include <Shared/Config/DataTypes.hpp>
#include <iostream>
#include <memory>
#include <vector>

namespace cfg
{
class ConfigSource;
class Config;

namespace detail
{

template<typename DataType>
struct ValueEntry
{
	ValueEntry() :
		valid(false),
		value()
	{
	}

	bool valid;
	DataType value;
};

template<typename KeyType>
using Cache = std::vector<ValueEntry<typename KeyType::DataType> >;

template<typename KeyType>
struct CacheGetter
{
	static Cache<KeyType> & get(const Config & configCache) = delete;
};

template<>
struct CacheGetter<StringKey>
{
	static Cache<StringKey> & get(const Config & configCache);
};

template<>
struct CacheGetter<BoolKey>
{
	static Cache<BoolKey> & get(const Config & configCache);
};

template<>
struct CacheGetter<IntKey>
{
	static Cache<IntKey> & get(const Config & configCache);
};

template<>
struct CacheGetter<FloatKey>
{
	static Cache<FloatKey> & get(const Config & configCache);
};

}

class Config
{
public:
	Config();
	virtual ~Config();

	void setConfigSource(std::shared_ptr<ConfigSource> config);
	ConfigSource * getConfigSource() const;

	void clearCache()
	{
		myCacheString.clear();
		myCacheBool.clear();
		myCacheInt.clear();
		myCacheFloat.clear();
	}

	template<typename KeyType>
	typename KeyType::DataType get(const KeyType & key) const
	{
		return key.onGet(*this);
	}

	template<typename KeyType>
	void set(const KeyType & key, typename KeyType::DataType value)
	{
		key.onSet(*this, std::move(value));
	}

	template<typename KeyType>
	typename KeyType::DataType getCachedValue(const KeyType & key) const
	{
		auto & cache = getCache<KeyType>();

		if (key.getIndex() >= cache.size())
		{
			cache.resize(key.getIndex() + 1);
		}

		auto & entry = cache[key.getIndex()];

		if (!entry.valid)
		{
			Value readValue = myConfigSource->readValue(key.getName());
			if (readValue.type == Value::Type::Missing)
			{
				std::cerr << "Failed to find config value entry " << key.getName() << std::endl;
			}
			entry.value = KeyType::convertTo(readValue);
			entry.valid = true;
		}

		return entry.value;
	}

	template<typename KeyType>
	void setCachedValue(const KeyType & key, typename KeyType::DataType value)
	{
		clearSingleCacheEntry(key.getIndex());

		auto & cache = getCache<KeyType>();

		if (key.getIndex() >= cache.size())
		{
			cache.resize(key.getIndex() + 1);
		}

		auto & entry = cache[key.getIndex()];

		if (!entry.valid)
		{
			entry.value = value;
			entry.valid = true;
		}

		myConfigSource->writeValue(key.getName(), KeyType::convertFrom(std::move(value)));
	}

private:

	void clearSingleCacheEntry(std::size_t index);

	template<typename KeyType>
	void unsetCachedValue(std::size_t index)
	{
		auto & cache = getCache<KeyType>();
		if (index < cache.size())
		{
			cache[index].valid = false;
		}
	}

	template<typename KeyType>
	detail::Cache<KeyType> & getCache() const
	{
		return detail::CacheGetter<KeyType>::get(*this);
	}

	bool myHasConfigSource;
	std::shared_ptr<ConfigSource> myConfigSource;

	mutable detail::Cache<StringKey> myCacheString;
	mutable detail::Cache<BoolKey> myCacheBool;
	mutable detail::Cache<IntKey> myCacheInt;
	mutable detail::Cache<FloatKey> myCacheFloat;

	template<typename KeyType>
	friend struct detail::CacheGetter;
};

}

#endif
