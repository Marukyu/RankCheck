#ifndef SRC_SHARED_UTILS_STATICKEY_HPP_
#define SRC_SHARED_UTILS_STATICKEY_HPP_

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * Template parameter only serves to generate a separate key cache for each subtype.
 */
template<typename T>
class StaticKey
{
public:

	using ID = std::size_t;

	static constexpr ID invalidIndex = std::numeric_limits<ID>::max();

	StaticKey() :
		index(invalidIndex),
		key("")
	{
	}

	StaticKey(std::string key) :
		index(acquireKeyIndex(key)),
		key(std::move(key))
	{
	}

	StaticKey(StaticKey<T> && key) = default;
	StaticKey(const StaticKey<T> & key) = default;

	StaticKey<T> & operator=(StaticKey<T> && key) = default;
	StaticKey<T> & operator=(const StaticKey<T> & key) = default;

	std::string getName() const
	{
		return key;
	}

	ID getIndex() const
	{
		return index;
	}

	bool isValid() const
	{
		return index != invalidIndex;
	}

	static StaticKey<T> fromIndex(ID index)
	{
		if (index < KeyCache::getInstance().entries.size())
		{
			return KeyCache::getInstance().entries[index];
		}
		else
		{
			return StaticKey<T>();
		}
	}

private:

	StaticKey(std::string key, ID index) :
		index(index),
		key(std::move(key))
	{
	}

	ID index;
	std::string key;

	struct KeyCache
	{
		std::unordered_map<std::string, ID> map;
		std::vector<StaticKey<T>> entries;

		static KeyCache & getInstance()
		{
			static KeyCache instance;
			return instance;
		}

	private:
		KeyCache() = default;
	};

	static ID acquireKeyIndex(std::string key)
	{
		auto it = KeyCache::getInstance().map.find(key);

		if (it == KeyCache::getInstance().map.end())
		{
			ID index = KeyCache::getInstance().entries.size();
			KeyCache::getInstance().map.insert(std::make_pair(key, index));
			KeyCache::getInstance().entries.push_back(StaticKey<T>(key, index));
			return index;
		}

		return it->second;
	}
};

#endif
