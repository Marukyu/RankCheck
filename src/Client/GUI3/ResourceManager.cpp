/*
 * ResourceManager.cpp
 *
 *  Created on: Jul 9, 2015
 *      Author: marukyu
 */

#include <Client/GUI3/ResourceManager.hpp>
#include <algorithm>

namespace gui3
{

namespace res
{

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

Resource::Resource(std::string name) :
	myName(std::move(name))
{
}

const std::string & Resource::getName() const
{
	return myName;
}

Ptr<Data> ResourceManager::acquireData(std::string dataName)
{
	return nullptr;
}

Ptr<Image> ResourceManager::acquireImage(std::string imageName)
{
	return nullptr;
}

Ptr<Font> ResourceManager::acquireFont(std::string fontName)
{
	return nullptr;
}

Data::Data(std::string name) :
	Resource(std::move(name))
{
}

Image::Image(std::string name) :
	Resource(std::move(name))
{
}

bool Image::isOnMainTexture() const
{
	return getTexturePage() == 0;
}

Font::Font(std::string name) :
		Resource(std::move(name))
{
}

bool Font::isOnMainTexture() const
{
	return getTexturePage() == 0;
}

void ResourceManager::cleanUpBeforeExit()
{
}

}
}
