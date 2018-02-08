#ifndef SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_TINTEDBOX_HPP_
#define SRC_CLIENT_GUI3_RENDERING_PRIMITIVES_TINTEDBOX_HPP_

#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <SFML/Graphics/Color.hpp>

namespace gui3
{
namespace primitives
{

class TintedBox : public Box
{
public:
	TintedBox(Box box, sf::Color tintColor);
	
	sf::Color getTintColor() const;

private:

	sf::Color myTintColor;
};

}
}

#endif
