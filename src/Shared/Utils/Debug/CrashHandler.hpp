#ifndef SRC_SHARED_UTILS_DEBUG_CRASHHANDLER_HPP_
#define SRC_SHARED_UTILS_DEBUG_CRASHHANDLER_HPP_

#include <functional>
#include <map>

class CrashHandler
{
public:

	using Func = std::function<void()>;

	static CrashHandler & getInstance();

	void registerSegfaultHandler(Func func);

	void registerSignalHandler(int sig, Func func);

private:

	static void handleSignal(int sig);

	CrashHandler();

	std::map<int, Func> handlers;
};

#endif
