#ifndef SRC_CLIENT_GUI3_EVENTS_KEY_HPP_
#define SRC_CLIENT_GUI3_EVENTS_KEY_HPP_

#include <SFML/Config.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace gui3
{

class Key
{
public:

	static const Key NONE;

	Key();

	bool isControl() const;
	bool isShift() const;
	bool isAlt() const;
	bool isMeta() const;

	bool operator==(const Key & key) const;
	bool operator!=(const Key & key) const;
	bool operator<(const Key & key) const;

	static Key fromSFML(sf::Keyboard::Key key);
	static sf::Keyboard::Key toSFML(Key key);

	static Key fromInt32(sf::Int32 code);
	static sf::Int32 toInt32(Key key);

private:

	sf::Int32 code;
};

}

#endif
