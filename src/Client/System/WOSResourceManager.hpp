#ifndef SRC_CLIENT_SYSTEM_WOSRESOURCEMANAGER_HPP_
#define SRC_CLIENT_SYSTEM_WOSRESOURCEMANAGER_HPP_

#include <Client/Graphics/TexturePacker.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct BitmapFont;
namespace sf
{
class Texture;
}

class Package;

class TexturePacker;

class WOSResourceManager : public gui3::res::ResourceManager
{
public:
	static const std::string IMAGE_NAME_WHITE_PIXEL;

	WOSResourceManager();
	virtual ~WOSResourceManager();

	/**
	 * Assigns a package to be managed by this resource manager.
	 */
	void setPackage(std::unique_ptr<Package> package);

	/**
	 * Gets a handle to the specified data file. Returns a null pointer if no such data was found.
	 */
	virtual gui3::Ptr<gui3::res::Data> acquireData(std::string dataName) override;

	/**
	 * Gets a handle to the specified image. Returns a null pointer if the image failed to load.
	 */
	virtual gui3::Ptr<gui3::res::Image> acquireImage(std::string imageName) override;

	/**
	 * Gets a handle to the specified font. Returns a null pointer if the font failed to load.
	 */
	virtual gui3::Ptr<gui3::res::Font> acquireFont(std::string fontName) override;

	/**
	 * Returns one of the textures containing loaded images.
	 */
	const sf::Texture * getTexture(std::size_t page) const;

	/**
	 * Frees all currently unused resources.
	 */
	void collectGarbage();

	/**
	 * Safely deallocates all resources immediately before the last window is closed.
	 */
	void cleanUpBeforeExit() override;

private:

	class Data : public gui3::res::Data
	{
	public:
		Data(std::string name, std::vector<char> data);
		virtual ~Data();

		virtual const char * getData() const override;
		virtual std::size_t getDataSize() const override;

	private:

		std::vector<char> myData;
	};

	class Image : public gui3::res::Image
	{
	public:
		Image(std::string name, TexturePacker::Handle handle, std::size_t page);
		virtual ~Image();

		virtual sf::FloatRect getTextureRect() const override;
		virtual std::size_t getTexturePage() const override;

	private:

		TexturePacker::Handle myHandle;
		std::size_t myTexturePage;
	};

	class Font : public gui3::res::Font
	{
	public:
		Font(std::string name, std::unique_ptr<BitmapFont> font, gui3::Ptr<gui3::res::Image> image);
		virtual ~Font();

		virtual const BitmapFont & getFont() const override;
		virtual std::size_t getTexturePage() const override;

	private:

		std::unique_ptr<BitmapFont> myFont;
		gui3::Ptr<gui3::res::Image> myImage;
	};

	template <typename ResourceType>
	void runGC(std::map<std::string, gui3::Ptr<ResourceType> > & map)
	{
		for (auto it = map.begin(); it != map.end(); )
		{
			if (it->second.isUnique())
			{
				it = map.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	std::string normalizeResourceName(std::string resourceName) const;

	gui3::Ptr<gui3::res::Image> addImageResource(std::string imageName, const sf::Image & image);

	std::unique_ptr<Package> myPackage;

	std::map<std::string, gui3::Ptr<gui3::res::Data> > myDatas;
	std::map<std::string, gui3::Ptr<gui3::res::Image> > myImages;
	std::map<std::string, gui3::Ptr<gui3::res::Font> > myFonts;

	std::vector<std::unique_ptr<TexturePacker> > myTexturePackers;
};

#endif /* SRC_CLIENT_SYSTEM_WOSRESOURCEMANAGER_HPP_ */
