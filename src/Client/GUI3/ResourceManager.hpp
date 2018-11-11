/*
 * ResourceManager.hpp
 *
 *  Created on: Jul 9, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_RESOURCEMANAGER_HPP_
#define SRC_CLIENT_GUI3_RESOURCEMANAGER_HPP_

#include <Client/GUI3/Types.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cstddef>
#include <string>

struct BitmapFont;

namespace gui3
{

namespace res
{

class Resource
{
public:
	Resource(std::string name);
	virtual ~Resource() = default;

	const std::string & getName() const;
private:
	std::string myName;
};

class Data : public Resource
{
public:
	Data(std::string name);
	virtual ~Data() = default;

	virtual const char * getData() const = 0;
	virtual std::size_t getDataSize() const = 0;
};

class Image : public Resource
{
public:
	Image(std::string name);
	virtual ~Image() = default;

	virtual sf::FloatRect getTextureRect() const = 0;
	virtual std::size_t getTexturePage() const = 0;
	bool isOnMainTexture() const;
};

class Font : public Resource
{
public:
	Font(std::string name);
	virtual ~Font() = default;

	virtual const BitmapFont & getFont() const = 0;
	virtual std::size_t getTexturePage() const = 0;
	bool isOnMainTexture() const;
};

class ResourceManager
{
public:

	ResourceManager();

	virtual ~ResourceManager();

	/**
	 * Gets a handle to the specified data file. Returns a null pointer if no such data was found.
	 */
	virtual Ptr<Data> acquireData(std::string dataName);

	/**
	 * Gets a handle to the specified image. Returns a null pointer if the image failed to load.
	 */
	virtual Ptr<Image> acquireImage(std::string imageName);

	/**
	 * Gets a handle to the specified font. Returns a null pointer if the font failed to load.
	 */
	virtual Ptr<Font> acquireFont(std::string fontName);

	/**
	 * Called immediately before the last window is closed.
	 */
	virtual void cleanUpBeforeExit();
};

}

}

#endif
