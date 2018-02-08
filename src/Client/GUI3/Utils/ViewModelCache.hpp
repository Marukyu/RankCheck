#ifndef SRC_CLIENT_GUI3_UTILS_VIEWMODELCACHE_HPP_
#define SRC_CLIENT_GUI3_UTILS_VIEWMODELCACHE_HPP_

#include <Client/GUI3/Types.hpp>
#include <cstddef>
#include <map>

namespace gui3
{

/**
 * Widget cache for view models (such as TableViewModel).
 */
template<typename T>
class ViewModelCache
{
public:

	using KeyType = std::size_t;
	using Map = std::map<KeyType, Ptr<T> >;
	using Iterator = typename Map::const_iterator;

	ViewModelCache<T>()
	{
	}

	void clear()
	{
		myCache.clear();
	}

	template<typename ... Args>
	Ptr<T> make(KeyType key, Args &&... args)
	{
		Ptr<T> ptr = gui3::make<T>(std::forward<Args>(args)...);

		if (myCache.count(key) == 0)
		{
			myCache[key] = ptr;
		}

		return ptr;
	}

	Ptr<T> get(KeyType key) const
	{
		auto it = myCache.find(key);

		if (it == myCache.end())
		{
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}

	bool has(KeyType key) const
	{
		return get(key) != nullptr;
	}

	void remove(KeyType key)
	{
		myCache.erase(key);
	}

	Iterator begin() const
	{
		return myCache.begin();
	}

	Iterator end() const
	{
		return myCache.end();
	}

private:

	Map myCache;
};

}

#endif
