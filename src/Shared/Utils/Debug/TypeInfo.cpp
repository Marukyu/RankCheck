#include <Shared/Utils/Debug/TypeInfo.hpp>
#include <string>

#ifdef __GNUC__

#include <cxxabi.h>
#include <stdlib.h>

std::string cxaDemangle(std::string mangledName)
{
	int status = 0;
	char * symbol = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);

	if (symbol != nullptr)
	{
		std::string ret = symbol;
		std::free(symbol);
		return ret;
	}
	else
	{
		return mangledName;
	}
}

#else

std::string cxaDemangle(std::string mangledName)
{
	return mangledName;
}

#endif
