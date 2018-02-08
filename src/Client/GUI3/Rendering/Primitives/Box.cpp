#include <Client/GUI3/Rendering/Primitives/Box.hpp>

namespace gui3
{
namespace primitives
{

Box::Box(sf::FloatRect rect, int flags, Shape shape) :
	myRect(rect),
	myFlags(flags),
	myShape(shape)
{
}

sf::FloatRect Box::getRect() const
{
	return myRect;
}

bool Box::testFlag(Flags flag) const
{
	return myFlags & flag;
}

Box::Shape Box::getShape() const
{
	return myShape;
}

}
}
