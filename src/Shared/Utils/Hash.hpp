#ifndef HASH_HPP
#define HASH_HPP

#include <SFML/Config.hpp>

// cross platform data hashing function.

sf::Uint32 dataHash32(const char * data, unsigned int length);
sf::Uint64 dataHash64(const char * data, unsigned int length);

#endif

