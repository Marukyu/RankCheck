#include <Shared/Config/Config.hpp>
#include <Shared/Config/NullConfig.hpp>

namespace cfg
{

namespace detail
{

Cache<StringKey> & CacheGetter<StringKey>::get(const Config & configCache)
{
	return configCache.myCacheString;
}
Cache<BoolKey> & CacheGetter<BoolKey>::get(const Config & configCache)
{
	return configCache.myCacheBool;
}
Cache<IntKey> & CacheGetter<IntKey>::get(const Config & configCache)
{
	return configCache.myCacheInt;
}
Cache<FloatKey> & CacheGetter<FloatKey>::get(const Config & configCache)
{
	return configCache.myCacheFloat;
}

}

Config::Config()
{
	setConfigSource(nullptr);
}

Config::~Config()
{
}

void Config::setConfigSource(std::shared_ptr<ConfigSource> config)
{
	myHasConfigSource = (config != nullptr);

	if (myHasConfigSource)
	{
		myConfigSource = config;
	}
	else
	{
		myConfigSource = std::make_shared<NullConfig>();
	}
}

ConfigSource* Config::getConfigSource() const
{
	return myHasConfigSource ? myConfigSource.get() : nullptr;
}

void Config::clearSingleCacheEntry(std::size_t index)
{
	unsetCachedValue<StringKey>(index);
	unsetCachedValue<BoolKey>(index);
	unsetCachedValue<IntKey>(index);
	unsetCachedValue<FloatKey>(index);
}

}
