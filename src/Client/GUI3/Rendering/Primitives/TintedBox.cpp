#include <Client/GUI3/Rendering/Primitives/TintedBox.hpp>

namespace gui3
{
namespace primitives
{

TintedBox::TintedBox(Box box, sf::Color tintColor) :
	Box(box),
	myTintColor(tintColor)
{
}

sf::Color TintedBox::getTintColor() const
{
	return myTintColor;
}

}
}
