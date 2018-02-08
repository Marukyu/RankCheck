#ifndef SRC_CLIENT_GUI3_EVENTS_KEYEVENT_HPP_
#define SRC_CLIENT_GUI3_EVENTS_KEYEVENT_HPP_

#include <Client/GUI3/Events/Key.hpp>
#include <SFML/Config.hpp>
#include <cassert>

namespace gui3
{

class KeyEvent
{
public:

	enum Type
	{
		/**
		 * Empty event mask.
		 */
		None = 0,

		/**
		 * Called when a key is pressed.
		 */
		Press = 1 << 0,

		/**
		 * Called when a key is released.
		 */
		Release = 1 << 1,

		/**
		 * Called when a unicode character is entered.
		 */
		Input = 1 << 2,

		/**
		 * Full event mask (for event forwarding).
		 */
		Any = 0x7fffffff
	};

	KeyEvent(Type type, Key key) :
		type(type),
		key(key),
		character(0)
	{
		assert(type != Input);
	}

	KeyEvent(Type type, sf::Uint32 character) :
		type(type),
		key(),
		character(character)
	{
		assert(type == Input);
	}

	const Type type;

	const Key key;
	const sf::Uint32 character;
};

}

#endif
