#ifndef SRC_CLIENT_GUI3_TYPES_HPP_
#define SRC_CLIENT_GUI3_TYPES_HPP_

#include <cstddef>
#include <memory>
#include <type_traits>

namespace gui3
{

template<typename T>
class Ptr
{
private:

	using NullPointerType = decltype(nullptr);

public:

	Ptr() = default;
	~Ptr() = default;

	Ptr(NullPointerType) :
		myPointer(nullptr)
	{
	}

	Ptr(const Ptr<T> &) = default;
	Ptr(Ptr<T> &&) = default;

	Ptr<T> & operator =(const Ptr<T> &) = default;
	Ptr<T> & operator =(Ptr<T> &&) = default;

	template<typename T2, typename = typename std::enable_if<std::is_convertible<T2*, T*>::value>::type>
	Ptr(const Ptr<T2> & pointer) noexcept : myPointer(pointer.myPointer)
	{
	}

	Ptr<T> & operator =(NullPointerType)
	{
		myPointer = nullptr;
		return *this;
	}

	T * get() const
	{
		return myPointer.get();
	}

	T & operator*() const
	{
		return *get();
	}

	T * operator->() const
	{
		return get();
	}

	explicit operator bool() const
	{
		return get() != nullptr;
	}

	bool operator ==(const Ptr<T> & other) const
	{
		return get() == other.get();
	}

	bool operator !=(const Ptr<T> & other) const
	{
		return get() != other.get();
	}

	bool operator ==(NullPointerType) const
	{
		return get() == nullptr;
	}

	bool operator !=(NullPointerType) const
	{
		return get() != nullptr;
	}

	template<typename ... Args>
	void make(Args && ... args)
	{
		myPointer = std::make_shared<T>(std::forward<Args>(args)...);
	}

	std::size_t ownershipCount() const
	{
		return myPointer.use_count();
	}

	bool isUnique() const
	{
		return myPointer.unique();
	}

private:

	std::shared_ptr<T> myPointer;

	template <typename T2>
	friend class Ptr;
};

template<typename T, typename ... Args>
Ptr<T> make(Args && ... args)
{
	Ptr<T> ptr;
	ptr.make(std::forward<Args>(args)...);
	return ptr;
}

}

#endif
