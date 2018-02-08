#ifndef SRC_SHARED_UTILS_DEBUGLOG_HPP_
#define SRC_SHARED_UTILS_DEBUGLOG_HPP_

#include <iostream>
#include <memory>
#include <string>

#define WOS_DEBUG_LOCATION writeDebugLocation(__FILE__, __LINE__, __PRETTY_FUNCTION__);

void writeDebugOutput(std::string log);
void writeDebugLocation(const char * fileName, unsigned int lineNumber, const char * functionName);

class DebugOutStream
{

private:

	class NewlinePrinter
	{
	public:
		~NewlinePrinter()
		{
			std::cout << std::endl;
		}
	};

public:

	DebugOutStream()
	{
		printer = std::make_shared<NewlinePrinter>();
	}

private:

	std::shared_ptr<NewlinePrinter> printer;
};

class NullOutStream
{
};

template <typename T>
DebugOutStream operator<<(DebugOutStream stream, T data)
{
	std::cout << data;
	return stream;
}

template <typename T>
NullOutStream operator<<(NullOutStream stream, T)
{
	return stream;
}


#ifdef WOS_DEBUG

inline DebugOutStream debug()
{
	return DebugOutStream();
}

#else

inline NullOutStream debug()
{
	return NullOutStream();
}

#endif


#endif
