#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>
#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>

static const std::string stringAlphabet = "abcdefghijklmnopqrstuvwxyz";
std::string toUppercase(std::string str);
std::string toLowercase(std::string str);

bool equalsIgnoreCase(const std::string & a, const std::string & b);
bool stringStartsWith(const std::string & string, const std::string & prefix);
bool stringEndsWith(const std::string & string, const std::string & suffix);

void splitString(std::string str, const std::string & separator, std::vector<std::string> & results, bool ignoreEmpty =
	false);

std::string getTimeString();

bool listFiles(std::string dir, std::vector<std::string> & vec, bool recursive = false, bool sorted = true,
	std::string prefix = "");
bool listDirectories(std::string dir, std::vector<std::string> & vec, bool recursive = false, bool sorted = true,
	std::string prefix = "");

bool fileExists(const std::string & filename);
bool isDirectory(const std::string & filename);

bool createDirectory(const std::string & path);
void createDirectories(const std::string & path);

std::string getFileExtension(const std::string & filename);
std::string removeFileExtension(const std::string & filename);

std::string extractFileName(const std::string & path);
std::string removeFileName(const std::string & path);

template<typename T>
sf::Rect<T> moveRect(sf::Rect<T> rect, sf::Vector2<T> off)
{
	rect.left += off.x;
	rect.top += off.y;
	return rect;
}
template<typename T>
sf::Rect<T> expandRect(sf::Rect<T> rect, sf::Vector2<T> exp)
{
	rect.left -= exp.x;
	rect.top -= exp.y;
	rect.width += exp.x * (T) 2;
	rect.height += exp.y * (T) 2;
	return rect;
}
template<typename T>
sf::Rect<T> expandRect(sf::Rect<T> rect, T exp)
{
	return expandRect(rect, {exp, exp});
}

std::string getByteSizeString(sf::Uint64 bytes);
std::string getSfTimeString(sf::Time time);
std::string getRoughSfTimeString(sf::Time time);

template<typename T>
sf::Rect<T> rectIntersect(sf::Rect<T> a, sf::Rect<T> b)
{
	T a_right = a.left + a.width, a_bottom = a.top + a.height;
	T b_right = b.left + b.width, b_bottom = b.top + b.height;

	T r_left = std::max(a.left, b.left);
	T r_top = std::max(a.top, b.top);
	T r_right = std::min(a_right, b_right);
	T r_bottom = std::min(a_bottom, b_bottom);

	return sf::Rect<T>(r_left, r_top, r_right - r_left, r_bottom - r_top);
}

bool pointInPoly(const std::vector<sf::Vector2f> & vertices, const sf::Vector2f & point);

#endif
