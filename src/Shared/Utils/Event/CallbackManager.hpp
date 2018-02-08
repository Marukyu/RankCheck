/*
 * CallbackManager.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#ifndef SRC_SHARED_UTILS_EVENT_CALLBACKMANAGER_HPP_
#define SRC_SHARED_UTILS_EVENT_CALLBACKMANAGER_HPP_

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

template<typename ... Args>
class CallbackManager
{
private:

	using ID = std::size_t;

public:

	using CallbackFunc = std::function<void(Args...)>;

	class Handle
	{
	public:
		Handle() = default;

		bool isValid() const
		{
			return !myManager.expired();
		}

		void remove() const
		{
			if (isValid())
			{
				(**myManager.lock()).removeCallback(myID);
			}
		}

	private:

		Handle(ID id, std::weak_ptr<CallbackManager<Args...>*> manager) :
			myID(id),
			myManager(manager)
		{
		}

		ID myID;
		std::weak_ptr<CallbackManager<Args...>*> myManager;

		friend class CallbackManager<Args...> ;
	};

	class ScopedHandle
	{
	public:
		ScopedHandle() = default;

		ScopedHandle(Handle handle) :
			myHandle(std::move(handle))
		{
		}

		~ScopedHandle()
		{
			myHandle.remove();
		}

		ScopedHandle(const ScopedHandle & scopedHandle) = delete;

		ScopedHandle(ScopedHandle && scopedHandle) :
			myHandle(std::move(scopedHandle.myHandle))
		{
		}

		ScopedHandle & operator=(const ScopedHandle & scopedHandle) = delete;

		ScopedHandle & operator=(const Handle & handle)
		{
			myHandle.remove();
			myHandle = handle;
			return *this;
		}

		ScopedHandle & operator=(ScopedHandle && scopedHandle)
		{
			myHandle.remove();
			myHandle = std::move(scopedHandle.myHandle);
			return *this;
		}

		void remove()
		{
			myHandle.remove();
		}

	private:

		Handle myHandle;
	};

	CallbackManager() :
		myIDCounter(0),
		mySelfPointer(std::make_shared<CallbackManager<Args...>*>(this))
	{
	}

	Handle addCallback(CallbackFunc function, int filter, int order)
	{
		CallbackStruct callback;

		ID id = myIDCounter++;

		callback.function = std::move(function);
		callback.filter = filter;
		callback.order = order;
		callback.id = id;

		auto it = std::upper_bound(myCallbacks.begin(), myCallbacks.end(), callback);
		myCallbacks.insert(it, std::move(callback));
		return Handle(id, mySelfPointer);
	}

	void fireCallback(int filter, Args ... args) const
	{
		auto callbacks = myCallbacks;
		for (const auto & callback : callbacks)
		{
			if (callback.filter & filter)
			{
				callback.function(args...);
			}
		}
	}

	void clearCallbacks()
	{
		myCallbacks.clear();
	}

private:

	struct CallbackStruct
	{
		CallbackFunc function;
		int filter;
		int order;
		ID id;

		bool operator<(const CallbackStruct & other) const
		{
			return order < other.order;
		}
	};

	void removeCallback(ID id)
	{
		for (auto it = myCallbacks.begin(); it != myCallbacks.end(); ++it)
		{
			if (it->id == id)
			{
				myCallbacks.erase(it);
				return;
			}
		}
	}

	std::vector<CallbackStruct> myCallbacks;
	ID myIDCounter;
	std::shared_ptr<CallbackManager<Args...>*> mySelfPointer;
};

#endif
