#include <Shared/Utils/Debug/StackTrace.hpp>
#include <sstream>

StackTrace StackTrace::generate(std::size_t ignoreFrames)
{
	StackTrace trace;

	// TODO: Missing stack trace implementation. Consider using the Boost stack trace library for this purpose.

	return trace;
}

StackTrace::Iterator StackTrace::begin() const
{
	return frames.begin();
}

StackTrace::Iterator StackTrace::end() const
{
	return frames.end();
}

std::size_t StackTrace::getFrameCount() const
{
	return frames.size();
}

const std::string& StackTrace::getFrame(std::size_t frameNumber) const
{
	static std::string emptyString;
	return frameNumber < frames.size() ? frames[frameNumber] : emptyString;
}

std::string StackTrace::toString() const
{
	std::ostringstream str;
	for (const std::string & frame : frames)
	{
		str << frame << '\n';
	}
	return str.str();
}
