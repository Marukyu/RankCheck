#include <Shared/Utils/Error.hpp>

Error::Error() noexcept
{
	stackTrace = StackTrace::generate(0);
	errorText = "Error";
	fullMessage = stackTrace.toString();
}

Error::Error(std::string errorText) noexcept
{
	stackTrace = StackTrace::generate(0);
	this->errorText = errorText;
	fullMessage = errorText + ":\n" + stackTrace.toString();
}

Error::~Error() noexcept
{
}

const StackTrace& Error::getStackTrace() const noexcept
{
	return stackTrace;
}

const std::string& Error::getErrorText() const noexcept
{
	return errorText;
}

const char* Error::what() const noexcept
{
	return fullMessage.c_str();
}
