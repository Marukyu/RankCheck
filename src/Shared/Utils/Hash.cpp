#include "Shared/Utils/Hash.hpp"

sf::Uint32 dataHash32(const char * data, unsigned int length)
{
	sf::Uint32 hash = 0x9e3779b9;
	for (unsigned int i = 0; i < length; ++i)
	{
		hash += *(data + i) * 31 ^ (length - (i + 1));
		hash = ((hash << 6) ^ (hash >> 2)) + hash;
	}
	return hash;
}

sf::Uint64 dataHash64(const char * data, unsigned int length)
{
	sf::Uint64 hash = 0x79b9128c9e3786b3u;
	for (unsigned int i = 0; i < length; ++i)
	{
		hash += *(data + i) * 63 ^ (length - (i + 1));
		hash = ((hash << 6) ^ (hash >> 2)) + hash;
	}
	return hash;
}
