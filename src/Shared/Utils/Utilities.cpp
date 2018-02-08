#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <Shared/Utils/OSDetect.hpp>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ctime>
#include <deque>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

std::string toUppercase(std::string str)
{
	for (std::string::iterator it = str.begin(); it != str.end(); ++it)
		*it = toupper(*it);
	return str;
}
std::string toLowercase(std::string str)
{
	for (std::string::iterator it = str.begin(); it != str.end(); ++it)
		*it = tolower(*it);
	return str;
}

bool equalsIgnoreCase(const std::string & a, const std::string & b)
{
	std::size_t size = a.size();

	if (size != b.size())
	{
		return false;
	}

	for (std::size_t i = 0; i < size; ++i)
	{
		if (tolower(a[i]) != tolower(b[i]))
		{
			return false;
		}
	}
	return true;
}

bool stringStartsWith(const std::string& string, const std::string& prefix)
{
	if (string.length() >= prefix.length())
	{
		return string.compare(0, prefix.length(), prefix) == 0;
	}
	else
	{
		return false;
	}
}

bool stringEndsWith(const std::string& string, const std::string& suffix)
{
	if (string.length() >= suffix.length())
	{
		return string.compare(string.length() - suffix.length(), suffix.length(), suffix) == 0;
	}
	else
	{
		return false;
	}
}

void splitString(std::string str, const std::string & separator, std::vector<std::string> & results, bool ignoreEmpty)
{
	results.clear();
	std::size_t found = str.find_first_of(separator);

	while (found != std::string::npos)
	{
		if (found > 0)
			results.push_back(str.substr(0, found));
		else if (!ignoreEmpty)
			results.push_back(std::string());
		str = str.substr(found + 1);
		found = str.find_first_of(separator);
	}
	if (str.length() > 0)
		results.push_back(str);
}

std::string getTimeString()
{
	std::time_t rawtime;
	std::tm * timeinfo;
	char buffer[80];
	std::string ret;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	ret.assign(buffer, strftime(buffer, 80, "%X", timeinfo));

	return ret;
}

bool listFiles(std::string dir, std::vector<std::string> & vec, bool recursive, bool sorted, std::string prefix)
{
	try
	{
		Poco::File directory(dir);

		if (!directory.isDirectory())
		{
			return false;
		}

		for (Poco::DirectoryIterator it(directory); it != Poco::DirectoryIterator(); ++it)
		{
			std::string filename = Poco::Path(it->path()).getFileName();
			if (it->isDirectory())
			{
				if (recursive)
				{
					std::string next = dir + Poco::Path::separator() + filename;
					std::string newPrefix = prefix + filename + Poco::Path::separator();
					listFiles(next, vec, true, sorted, newPrefix);
				}
			}
			else if (it->isFile())
			{
				if (sorted)
				{
					vec.insert(std::lower_bound(vec.begin(), vec.end(), filename), filename);
				}
				else
				{
					vec.push_back(prefix + filename);
				}
			}
		}

		return true;
	}
	catch (std::exception & ex)
	{
		return false;
	}
}

bool listDirectories(std::string dir, std::vector<std::string> & vec, bool recursive, bool sorted, std::string prefix)
{
	try
	{
		Poco::File directory(dir);

		if (!directory.isDirectory())
		{
			return false;
		}

		for (Poco::DirectoryIterator it(directory); it != Poco::DirectoryIterator(); ++it)
		{
			std::string filename = Poco::Path(it->path()).getFileName();
			if (it->isDirectory())
			{
				if (sorted)
				{
					vec.insert(std::lower_bound(vec.begin(), vec.end(), filename), filename);
				}
				else
				{
					vec.push_back(prefix + filename);
				}

				if (recursive)
				{
					std::string next = dir + Poco::Path::separator() + filename;
					std::string newPrefix = prefix + filename + Poco::Path::separator();
					listDirectories(next, vec, true, sorted, newPrefix);
				}
			}
		}

		return true;
	}
	catch (std::exception & ex)
	{
		return false;
	}
}

bool fileExists(const std::string & filename)
{
	try
	{
		return Poco::File(filename).exists();
	}
	catch (std::exception & ex)
	{
		return false;
	}
}

bool isDirectory(const std::string & filename)
{
	try
	{
		return fileExists(filename) && Poco::File(filename).isDirectory();
	}
	catch (std::exception & ex)
	{
		return false;
	}
}

bool createDirectory(const std::string& path)
{
	return Poco::File(path).createDirectory();
}

void createDirectories(const std::string& path)
{
	return Poco::File(path).createDirectories();
}

std::string getFileExtension(const std::string & filename)
{
	// no dot found in file name.
	if (filename.find_last_of(".") == std::string::npos)
		return "";

	std::string extension = filename.substr(filename.find_last_of("."));

	// last dot found within path name, so no dot in file name.
	if (extension.find_first_of("/\\") != std::string::npos)
		return "";

	return extension;
}
std::string removeFileExtension(const std::string & filename)
{
	// no dot found in file name.
	if (filename.find_last_of(".") == std::string::npos)
		return filename;

	std::string filestem = filename.substr(0, filename.find_last_of("."));
	std::string extension = filename.substr(filename.find_last_of("."));

	// last dot found within path name, no dot to be removed in file name.
	if (extension.find_first_of("/\\") != std::string::npos)
		return filename;

	return filestem;
}

std::string extractFileName(const std::string& path)
{
	std::size_t found = path.find_last_of("/\\");

	if (found == std::string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(found + 1);
	}
}

std::string removeFileName(const std::string& path)
{
	std::size_t found = path.find_last_of("/\\");

	if (found == std::string::npos)
	{
		return "";
	}
	else
	{
		return path.substr(0, found);
	}
}

std::string getByteSizeString(sf::Uint64 bytes)
{
	if (bytes < 1000)
	{
		return cNtoS(bytes) + " B";
	}
	else if (bytes < 1000000)
	{
		return cNtoS(round<double>(bytes / 1000.0, 1)) + " KB";
	}
	else if (bytes < 1000000000)
	{
		return cNtoS(round<double>(bytes / 1000000.0, 1)) + " MB";
	}
	else
	{
		return cNtoS(round<double>(bytes / 1000000000.0, 1)) + " GB";
	}
}

std::string fillStringWithChar(std::string str, char chr, std::size_t targetSize)
{
	if (str.size() < targetSize)
		return std::string(targetSize - str.size(), chr) + str;
	else
		return str;
}

std::string getSfTimeString(sf::Time time)
{
	std::ostringstream str;

	str.precision(2);
	str << std::fixed;

	if (time < sf::milliseconds(1))
	{
		str << time.asMicroseconds() << "us";
	}
	else if (time < sf::seconds(1))
	{
		str << round(time.asSeconds() * 1000.f, 2) << "ms";
	}
	else if (time < sf::seconds(60))
	{
		str << round(time.asSeconds(), 2) << "s";
	}
	else if (time < sf::seconds(60 * 60))
	{
		str << int(time.asSeconds()) / 60 << ":" << fillStringWithChar(cNtoS(int(time.asSeconds()) % 60), '0', 2) << "."
			<< fillStringWithChar(cNtoS((time.asMilliseconds() / 10) % 100), '0', 2);
	}
	else
	{
		return getRoughSfTimeString(time);
	}

	return str.str();
}

std::string getRoughSfTimeString(sf::Time time)
{
	if (time > sf::seconds(60 * 60))
	{
		return cNtoS(int(time.asSeconds() / 60 / 60)) + ":"
			+ fillStringWithChar(cNtoS(int(time.asSeconds() / 60) % 60), '0', 2) + ":"
			+ fillStringWithChar(cNtoS(int(time.asSeconds()) % 60), '0', 2);
	}
	else
	{
		return cNtoS(int(time.asSeconds() / 60) % 60) + ":"
			+ fillStringWithChar(cNtoS(int(time.asSeconds()) % 60), '0', 2);
	}
}

bool pointInPoly(const std::vector<sf::Vector2f> & vertices, const sf::Vector2f & point)
{
	bool ret = false;
	for (std::size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++)
	{
		const sf::Vector2f & v1 = vertices[i], &v2 = vertices[j];
		if ((v1.y > point.y) != (v2.y > point.y) && point.x < (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x)
			ret = !ret;
	}
	return ret;
}
