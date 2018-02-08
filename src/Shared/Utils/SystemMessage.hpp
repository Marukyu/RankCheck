#ifndef SRC_SHARED_UTILS_SYSTEMMESSAGE_HPP_
#define SRC_SHARED_UTILS_SYSTEMMESSAGE_HPP_

#include <string>

class SystemMessage
{
public:

	static void showErrorMessage(std::string title, std::string text);

private:
	SystemMessage();
};

#endif
