#ifndef PATHS_HPP
#define PATHS_HPP

#include <string>

namespace paths
{
void createPaths();

std::string getPackageExtension();
std::string getContentExtension();
std::string getScriptExtension();
std::string getConfigExtension();

std::string getMainPath();

enum ConfigFile
{
	ConfigGame,		 // game and app configuration (video modes, game settings, keybinds...).
	ConfigModelEdit,	// model editor configuration.
};
std::string getConfigPath();
std::string getConfigFileName(ConfigFile file);
std::string getConfigFileName(std::string filename);

std::string getExecutableFileName();
std::string getExecutablePath();

std::string getDefaultDataPath();
std::string getDataPath(int fallback = 0);
std::string getAssetsFileName(bool defaultPath = false);

std::string getPluginPath();
std::string getPluginSourcePath();
}

#endif
