#include <Client/System/WOSApplication.hpp>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
	std::vector<std::string> args(argv, argv + argc);

	WOSApplication client;
	return client.run(args);
}
