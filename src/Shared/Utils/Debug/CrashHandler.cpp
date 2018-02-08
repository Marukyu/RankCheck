#include <Shared/Utils/Debug/CrashHandler.hpp>
#include <csignal>
#include <cstdlib>
#include <utility>

CrashHandler& CrashHandler::getInstance()
{
	static CrashHandler instance;
	return instance;
}

void CrashHandler::registerSegfaultHandler(Func func)
{
	registerSignalHandler(SIGSEGV, func);
}

void CrashHandler::registerSignalHandler(int sig, Func func)
{
	handlers[sig] = func;
	std::signal(sig, &CrashHandler::handleSignal);
}

void CrashHandler::handleSignal(int sig)
{
	auto & instance = CrashHandler::getInstance();

	auto it = instance.handlers.find(sig);
	if (it != instance.handlers.end())
	{
		it->second();
	}

	// Abort on specific signals.
	if (sig == SIGSEGV)
	{
		abort();
	}
}

CrashHandler::CrashHandler()
{
}

