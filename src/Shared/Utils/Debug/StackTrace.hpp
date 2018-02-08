#ifndef SRC_SHARED_UTILS_DEBUG_STACKTRACE_HPP_
#define SRC_SHARED_UTILS_DEBUG_STACKTRACE_HPP_

#include <cstddef>
#include <string>
#include <vector>

class StackTrace
{
public:

	using Iterator = std::vector<std::string>::const_iterator;

	StackTrace() = default;
	~StackTrace() = default;

	static StackTrace generate(std::size_t ignoreFrames = 1);

	Iterator begin() const;
	Iterator end() const;

	std::size_t getFrameCount() const;
	const std::string & getFrame(std::size_t frameNumber) const;

	std::string toString() const;

private:

	std::vector<std::string> frames;
};

#endif
