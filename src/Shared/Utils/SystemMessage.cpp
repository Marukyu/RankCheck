#include <Shared/External/TinyFileDialogs/tinyfiledialogs.h>
#include <Shared/Utils/SystemMessage.hpp>

void SystemMessage::showErrorMessage(std::string title, std::string text)
{
	tinyfd_messageBox(title.c_str(), text.c_str(), "ok", "error", 0);
}

SystemMessage::SystemMessage()
{
}

