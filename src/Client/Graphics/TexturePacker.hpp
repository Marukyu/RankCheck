#ifndef TEXTURE_PACKER_HPP
#define TEXTURE_PACKER_HPP

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>

// packs images densely in a texture.
class TexturePacker
{
public:

	typedef sf::Int32 NodeID;
	static const NodeID packFailure;

	TexturePacker();
	~TexturePacker();

	NodeID add(const sf::Image & image);
	void free(NodeID index);
	void clear();
	bool empty() const;

	sf::IntRect getImageRect(NodeID index) const;

	void setMinimumTextureSize(sf::Vector2u minimumSize);
	sf::Vector2u getMinimumTextureSize() const;

	const sf::Texture * getTexture() const;

	void setSmooth(bool smooth);
	bool isSmooth() const;

	class Handle
	{
	public:
		Handle() = default;
		Handle(TexturePacker * packer, NodeID id);
		~Handle();

		sf::IntRect getImageRect() const;

	private:

		class UniqueHandle
		{
		public:
			UniqueHandle(TexturePacker * packer, NodeID id);
			~UniqueHandle();

			TexturePacker * packer;
			NodeID id;
		};

		std::shared_ptr<UniqueHandle> myUniqueHandle;
	};

private:

	static const NodeID emptyNode, stubNode;

	struct Node : std::enable_shared_from_this<Node>
	{
		Node();

		std::shared_ptr<Node> sub1, sub2;
		sf::Vector2u pos;
		sf::Vector2u size;  // 0 = unlimited.
		NodeID id;

		std::shared_ptr<Node> add(const sf::Image & image, sf::Vector2u maxSize);
		sf::Vector2u getSizeBounds(sf::Vector2u currentMax = sf::Vector2u()) const;
	};

	bool createTransparentTexture(sf::Vector2u size);
	bool resizeToFit();

	std::shared_ptr<Node> myTree;
	std::vector<std::weak_ptr<Node> > myLookupTable;
	NodeID myIdCounter;

	std::unique_ptr<sf::Texture> myTexture;
	sf::Vector2u myMinimumSize;

};

#endif
