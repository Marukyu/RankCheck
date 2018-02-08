/*
 * LockableVector.hpp
 *
 *  Created on: Jul 16, 2015
 *      Author: marukyu
 */

#ifndef SRC_SHARED_UTILS_LOCKABLEVECTOR_HPP_
#define SRC_SHARED_UTILS_LOCKABLEVECTOR_HPP_

#include <vector>
#include <limits>
#include <cassert>
#include <iostream>
#include <Shared/Utils/MakeUnique.hpp>

template<typename T>
class LockableVector : public std::vector<T>
{
public:

	LockableVector() :
		myLockedVector(),
		myLockedIteratorCount(0)
	{
	}

	template<bool Reverse>
	class LockingIterator
	{
	public:
		LockingIterator(LockingIterator<Reverse> && other) = default;
		LockingIterator(const LockingIterator<Reverse> & other)
		{
			myParent = other.myParent;
			myIndex = other.myIndex;
			myParent->incrementLockedIteratorCount();
		}

		LockingIterator<Reverse> & operator=(LockingIterator<Reverse> && other) = default;
		LockingIterator<Reverse> & operator=(const LockingIterator<Reverse> & other)
		{
			other.myParent->incrementLockedIteratorCount();
			myParent->decrementLockedIteratorCount();

			myParent = other.myParent;
			myIndex = other.myIndex;
			return *this;
		}

		~LockingIterator()
		{
			myParent->decrementLockedIteratorCount();
		}

		const T & operator*() const
		{
			return *operator->();
		}

		const T * operator->() const
		{
			if (Reverse)
			{
				return myParent->getLockedItem(myIndex - 1);
			}
			else
			{
				return myParent->getLockedItem(myIndex);
			}
		}

		LockingIterator<Reverse> & operator++()
		{
			if (Reverse)
			{
				--myIndex;
			}
			else
			{
				++myIndex;
			}
			return *this;
		}

		LockingIterator<Reverse> operator++(int)
		{
			auto copy = *this;
			operator++();
			return copy;
		}

		bool operator==(const LockingIterator<Reverse> & other) const
		{
			return myIndex == other.myIndex;
		}

		bool operator!=(const LockingIterator<Reverse> & other) const
		{
			return myIndex != other.myIndex;
		}

	private:

		LockingIterator(const LockableVector * parent, std::size_t index)
		{
			myParent = parent;
			myIndex = index;
			myParent->incrementLockedIteratorCount();
		}

		const LockableVector * myParent;
		std::size_t myIndex;

		friend class LockableVector;
	};

	LockingIterator<false> lbegin() const
	{
		return LockingIterator<false>(this, 0);
	}

	LockingIterator<false> lend() const
	{
		return LockingIterator<false>(this, getLockedSize());
	}

	LockingIterator<true> lrbegin() const
	{
		return LockingIterator<true>(this, getLockedSize());
	}

	LockingIterator<true> lrend() const
	{
		return LockingIterator<true>(this, 0);
	}

	/**
	 * Call this just before making any changes to the vector.
	 */
	void invalidate()
	{
		if (myLockedIteratorCount > 0 && myLockedVector == nullptr)
		{
			myLockedVector = makeUnique<std::vector<T>>(*this);
		}
	}

private:

	void incrementLockedIteratorCount() const
	{
		myLockedIteratorCount++;
	}

	void decrementLockedIteratorCount() const
	{
		assert(myLockedIteratorCount > 0);

		myLockedIteratorCount--;

		if (myLockedIteratorCount == 0)
		{
			myLockedVector = nullptr;
		}
	}

	const T * getLockedItem(std::size_t index) const
	{
		if (myLockedVector != nullptr)
		{
			return &(*myLockedVector)[index];
		}
		else
		{
			return &std::vector<T>::operator[](index);
		}
	}

	std::size_t getLockedSize() const
	{
		if (myLockedVector != nullptr)
		{
			return myLockedVector->size();
		}
		else
		{
			return std::vector<T>::size();
		}
	}

	mutable std::unique_ptr<std::vector<T>> myLockedVector;
	mutable unsigned int myLockedIteratorCount;

	template<bool Reverse>
	friend class LockingIterator;
};

#endif
