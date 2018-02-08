#ifndef SRC_SHARED_UTILS_DEBUG_TYPEINFO_HPP_
#define SRC_SHARED_UTILS_DEBUG_TYPEINFO_HPP_

#include <string>
#include <typeinfo>

std::string cxaDemangle(std::string mangledName);

template<typename T>
class TypeInfo
{
public:

	TypeInfo() :
		typeInfo(typeid(T()))
	{
	}

	TypeInfo(const T & object) :
		typeInfo(typeid(object))
	{
	}

	std::string name() const
	{
		return cxaDemangle(typeInfo.name());
	}

private:

	const std::type_info & typeInfo;
};

#endif
