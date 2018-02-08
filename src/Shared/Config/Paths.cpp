#include "Shared/Utils/OSDetect.hpp"

#ifdef WOS_LINUX
#include <unistd.h>
#elif defined WOS_WINDOWS
#include <windows.h>
#endif

#include "Shared/Config/Paths.hpp"
#include "Shared/Utils/StrNumCon.hpp"

namespace paths
{

void createPaths()
{
	/*
	bfs::create_directories(getMainPath());
	bfs::create_directories(getConfigPath());
	bfs::create_directories(getPluginPath());
	bfs::create_directories(getPluginSourcePath());
	 */
	// TODO
}

std::string getPackageExtension()
{
	return ".wsp";
}

std::string getContentExtension()
{
	return ".wsc";
}

std::string getScriptExtension()
{
	return ".wss";
}

std::string getConfigExtension()
{
	return ".wsn";
}

std::string getMainPath()
{
#ifdef WOS_LINUX
	return getenv("HOME") + (std::string) "/.worldofsand";
#elif defined WOS_WINDOWS
	return getenv("APPDATA")+(std::string)"/Marukyu/WorldOfSand";
#endif
}

std::string getConfigPath()
{
	return getMainPath() + "/config";
	//return (bfs::path(getMainPath()) / bfs::path("config")).string();
}

std::string getConfigFileName(ConfigFile file)
{
	//bfs::path filename;
	std::string filename;

	switch (file)
	{
	case ConfigGame:
		filename = "game";
		break;
	case ConfigModelEdit:
		filename = "modeleditor";
		break;
	default:
		filename = "config" + cNtoS((int) file);
		break;
	}
	/*
	bfs::path ret(bfs::path(getConfigPath()) / filename);
	ret.replace_extension(getConfigExtension());
	return ret.string();
	 */

	return getConfigFileName(filename);
}

std::string getConfigFileName(std::string filename)
{
	return getConfigPath() + "/" + filename + getConfigExtension();
}

std::string getDefaultDataPath()
{
	//return bfs::current_path().string();
	return getMainPath();
}

std::string getExecutableFileName()
{
	char buffer[1024];
	int len;

#ifdef WOS_LINUX
	len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
#elif defined WOS_WINDOWS
	len = GetModuleFileName(0, buffer, sizeof(buffer)-1);
#endif

	if (len != -1 && len != 0)
	{
		buffer[len] = '\0';
		return std::string(buffer);
	}
	else
	{
		return "";
	}
}

std::string getExecutablePath()
{
	std::string exeName = getExecutableFileName();
	auto found = exeName.find_last_of('/');
	return found == std::string::npos ? exeName : exeName.substr(0, found);
}

std::string getDataPath(int fallback)
{
	/*
	 switch (fallback)
	 {
	 case 0: return bfs::current_path().string();
	 case 1: return getDefaultDataPath();
	 case 2: return ".";
	 default: return "";
	 }
	 */

	switch (fallback)
	{
	case 0:
		return getMainPath();
	case 1:
		return getDefaultDataPath();
	case 2:
		return getExecutablePath();
	case 3:
		return ".";
	default:
		return "";
	}
}

std::string getAssetsFileName(bool defaultPath)
{
	for (int f = 0;; ++f)
	{
		std::string fpath = defaultPath ? getDefaultDataPath() : getDataPath(f);
		std::string filename = fpath + "/assets" + getPackageExtension();

		if (fpath.empty())
		{
			break;
		}

		if (defaultPath /*|| (bfs::is_directory(fpath) && bfs::is_regular_file(filename)) */)
		{
			return filename;
		}
	}

	return "";
}

std::string getPluginPath()
{
	return getMainPath() + "/plugins";
}

std::string getPluginSourcePath()
{
	return getMainPath() + "/dev";
}

}   // namespace paths.

