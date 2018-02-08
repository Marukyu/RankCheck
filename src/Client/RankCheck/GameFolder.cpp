#include <Client/RankCheck/GameFolder.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/OSDetect.hpp>
#include <Shared/Utils/Utilities.hpp>

#ifdef WOS_WINDOWS
#include <windows.h>
#include <locale>
#endif

GameFolder& GameFolder::getInstance()
{
	static GameFolder instance;
	return instance;
}

void GameFolder::initWithConfig(cfg::Config& config)
{
	static cfg::String folderKey("rankcheck.awesomenauts.gameFolder");

	gameFolder = config.get(folderKey);

	if (!isDirectory(gameFolder + "/Data/Replays"))
	{
#ifdef WOS_WINDOWS
		HKEY hKey;
		RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Valve\\Steam", 0, KEY_READ, &hKey);
		CHAR szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError = RegQueryValueExA(hKey, "InstallPath", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		if (nError == ERROR_SUCCESS)
		{
			gameFolder = szBuffer + std::string("/steamapps/common/Awesomenauts");
		}

		if (!isDirectory(gameFolder + "/Data/Replays"))
		{
			throw Error("The selected Awesomenauts directory is not valid");
		}
#else
		throw Error("The selected Awesomenauts directory is not valid");
#endif
	}
}

std::string GameFolder::getFolder() const
{
	return gameFolder;
}

GameFolder::GameFolder()
{
}

