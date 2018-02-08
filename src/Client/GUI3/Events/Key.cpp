#include <Client/GUI3/Events/Key.hpp>

namespace gui3
{

const Key Key::NONE = Key();

Key::Key() :
	code(-2)
{
}

bool Key::isControl() const
{
	return *this == Key::fromSFML(sf::Keyboard::LControl) || *this == Key::fromSFML(sf::Keyboard::RControl);
}

bool Key::isShift() const
{
	return *this == Key::fromSFML(sf::Keyboard::LShift) || *this == Key::fromSFML(sf::Keyboard::RShift);
}

bool Key::isAlt() const
{
	return *this == Key::fromSFML(sf::Keyboard::LAlt) || *this == Key::fromSFML(sf::Keyboard::RAlt);
}

bool Key::isMeta() const
{
	return *this == Key::fromSFML(sf::Keyboard::LSystem) || *this == Key::fromSFML(sf::Keyboard::RSystem);
}

bool Key::operator ==(const Key& key) const
{
	return code == key.code;
}

bool Key::operator !=(const Key& key) const
{
	return code != key.code;
}

bool Key::operator <(const Key& key) const
{
	return code < key.code;
}

Key Key::fromSFML(sf::Keyboard::Key key)
{
	Key keyObject;
	keyObject.code = key;
	return keyObject;
}

sf::Keyboard::Key Key::toSFML(Key key)
{
	return (sf::Keyboard::Key)key.code;
}

Key Key::fromInt32(sf::Int32 code)
{
	Key key;
	key.code = code;
	return key;
}

sf::Int32 Key::toInt32(Key key)
{
	return key.code;
}

}
