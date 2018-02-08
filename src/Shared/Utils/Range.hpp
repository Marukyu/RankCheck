#ifndef SRC_SHARED_UTILS_RANGE_HPP_
#define SRC_SHARED_UTILS_RANGE_HPP_

template<typename T>
class Range
{
public:

	Range() :
		start(0),
		end(0)
	{
	}

	Range(T start, T end) :
		start(start),
		end(end)
	{
	}

	T start;
	T end;

	T size() const
	{
		return end - start;
	}

	T middle() const
	{
		return start + size() / 2;
	}

	/**
	 * Returns 1 if this range is to the "right" of the other range, -1 if it is to the "left", and 0 if the ranges
	 * intersect.
	 */
	int compare(Range<T> other) const
	{
		if (end <= other.start)
		{
			return -1;
		}
		else if (start >= other.end)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	/**
	 * Returns true if the specified value is contained within the range [start; end).
	 */
	bool contains(T value) const
	{
		return value >= start && value < end;
	}

	/**
	 * Merges the range with another range. If the two ranges do not intersect, fills the space between them.
	 */
	Range<T> merge(Range<T> other) const
	{
		return Range<T>(std::min(start, other.start), std::max(end, other.end));
	}
};

#endif
