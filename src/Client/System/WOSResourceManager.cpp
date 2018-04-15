#include <Client/Graphics/BitmapText.hpp>
#include <Client/System/WOSResourceManager.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <Shared/Content/Package.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <algorithm>
#include <utility>

const std::string WOSResourceManager::IMAGE_NAME_WHITE_PIXEL = "$white_pixel";

WOSResourceManager::WOSResourceManager()
{
}

WOSResourceManager::~WOSResourceManager()
{
}

void WOSResourceManager::setPackage(std::unique_ptr<Package> package)
{
	myPackage = std::move(package);
}

gui3::Ptr<gui3::res::Data> WOSResourceManager::acquireData(std::string dataName)
{
	dataName = normalizeResourceName(dataName);

	auto it = myDatas.find(dataName);
	if (it != myDatas.end())
	{
		return it->second;
	}

	std::vector<char> data;

	if (myPackage == nullptr || !myPackage->select(dataName))
	{
		return nullptr;
	}
	data = myPackage->getContentData();

	gui3::Ptr<gui3::res::Data> resource = gui3::make<Data>(dataName, std::move(data));
	myDatas.emplace(dataName, resource);
	return resource;
}

gui3::Ptr<gui3::res::Image> WOSResourceManager::acquireImage(std::string imageName)
{
	imageName = normalizeResourceName(imageName);

	auto it = myImages.find(imageName);
	if (it != myImages.end())
	{
		return it->second;
	}

	sf::Image image;

	if (imageName == WOSResourceManager::IMAGE_NAME_WHITE_PIXEL)
	{
		image.create(1, 1, sf::Color::White);
	}
	else
	{
		if (myPackage == nullptr || !myPackage->select(imageName))
		{
			return nullptr;
		}

		image.loadFromMemory(myPackage->getContentData().data(), myPackage->getContentSize());
		myPackage->deselect();
	}

	return addImageResource(imageName, image);
}

gui3::Ptr<gui3::res::Font> WOSResourceManager::acquireFont(std::string fontName)
{
	fontName = normalizeResourceName(fontName);

	// Check if font is in resource cache already.
	auto it = myFonts.find(fontName);
	if (it != myFonts.end())
	{
		// Return cached entry.
		return it->second;
	}

	// Select image file for font from package.
	if (myPackage == nullptr || !myPackage->select(fontName))
	{
		return nullptr;
	}

	sf::Image image;
	image.loadFromMemory(myPackage->getContentData().data(), myPackage->getContentSize());
	myPackage->deselect();

	std::unique_ptr<BitmapFont> font = std::unique_ptr<BitmapFont>(BitmapFont::generate(image));

	std::string fontImageName = "$font_image:" + fontName;
	gui3::Ptr<gui3::res::Image> imageResource = addImageResource(fontImageName, image);
	if (imageResource == nullptr)
	{
		return nullptr;
	}

	font->texture = getTexture(imageResource->getTexturePage());
	font->x = imageResource->getTextureRect().left;
	font->y = imageResource->getTextureRect().top;

	gui3::Ptr<gui3::res::Font> resource = gui3::make<Font>(fontName, std::move(font), imageResource);
	myFonts.insert(std::make_pair(fontName, resource));
	return resource;
}

WOSResourceManager::Data::Data(std::string name, std::vector<char> data) :
	gui3::res::Data(std::move(name)),
	myData(std::move(data))
{
}

WOSResourceManager::Data::~Data()
{
}

const char* WOSResourceManager::Data::getData() const
{
	return myData.data();
}

std::size_t WOSResourceManager::Data::getDataSize() const
{
	return myData.size();
}

WOSResourceManager::Image::Image(std::string name, TexturePacker::Handle handle, std::size_t page) :
	gui3::res::Image(std::move(name)),
	myHandle(handle),
	myTexturePage(page)
{
}

WOSResourceManager::Image::~Image()
{
}

sf::FloatRect WOSResourceManager::Image::getTextureRect() const
{
	return sf::FloatRect(myHandle.getImageRect());
}

std::size_t WOSResourceManager::Image::getTexturePage() const
{
	return myTexturePage;
}

WOSResourceManager::Font::Font(std::string name, std::unique_ptr<BitmapFont> font, gui3::Ptr<gui3::res::Image> image) :
	gui3::res::Font(std::move(name)),
	myFont(std::move(font)),
	myImage(image)
{
}

WOSResourceManager::Font::~Font()
{
}

const BitmapFont& WOSResourceManager::Font::getFont() const
{
	return *myFont;
}

std::size_t WOSResourceManager::Font::getTexturePage() const
{
	return myImage->getTexturePage();
}

std::string WOSResourceManager::normalizeResourceName(std::string resourceName) const
{
	if (resourceName.empty() || resourceName[0] == '$')
	{
		return resourceName;
	}

	// TODO: normalize name.
	return resourceName;
}

const sf::Texture* WOSResourceManager::getTexture(std::size_t page) const
{
	if (page < myTexturePackers.size())
	{
		return myTexturePackers[page]->getTexture();
	}
	else
	{
		return nullptr;
	}
}

void WOSResourceManager::collectGarbage()
{
	runGC(myDatas);
	runGC(myImages);
	runGC(myFonts);
}

void WOSResourceManager::cleanUpBeforeExit()
{
	// Texture packers need to be cleaned (so that handle destructor calls to "free()" no longer access texture data).
	// Texture packer instances must not be destroyed until the handles themselves are destroyed to avoid segfaults.
	for (const auto & texPacker : myTexturePackers)
	{
		texPacker->clear();
	}

	myDatas.clear();
	myImages.clear();
	myFonts.clear();
}

gui3::Ptr<gui3::res::Image> WOSResourceManager::addImageResource(std::string imageName, const sf::Image& image)
{
	TexturePacker::NodeID node = TexturePacker::packFailure;
	std::size_t page;

	// Try adding image to existing pages first.
	for (page = 0; page < myTexturePackers.size(); ++page)
	{
		node = myTexturePackers[page]->add(image);
		if (node != TexturePacker::packFailure)
		{
			break;
		}
	}

	// None of the existing pages accepted the image?
	if (node == TexturePacker::packFailure)
	{
		// Try adding another page to the end.
		myTexturePackers.push_back(makeUnique<TexturePacker>());
		myTexturePackers.back()->setSmooth(true);
		node = myTexturePackers.back()->add(image);

		// New page rejected image as well? Remove page and give up. Image is probably too big.
		if (node == TexturePacker::packFailure)
		{
			myTexturePackers.pop_back();
			return nullptr;
		}
	}

	// Create smart pointer to resource.
	auto handle = TexturePacker::Handle(myTexturePackers[page].get(), node);
	gui3::Ptr<gui3::res::Image> resource = gui3::make<Image>(imageName, handle, page);

	// Add weak pointer to resource cache.
	myImages.insert(std::make_pair(imageName, resource));

	return resource;
}
