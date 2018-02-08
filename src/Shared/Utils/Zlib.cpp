#include <zlib.h>
#include <iostream>

#include "Shared/Utils/Zlib.hpp"
#include "Shared/Utils/Endian.hpp"

namespace zu
{

bool compress(std::vector<char> & data, int level)
{
	std::vector<char> destBuf;
	bool ret = compressTransfer(data, destBuf, level);

	if (ret)
	{
		data = destBuf;
	}

	return ret;
}

bool decompress(std::vector<char> & data)
{
	std::vector<char> destBuf;
	bool ret = decompressTransfer(data, destBuf);

	if (ret)
	{
		data = destBuf;
	}

	return ret;
}

bool compressTransfer(const std::vector<char>& input, std::vector<char>& output, int level)
{
	int ret;

	// resize array to have sufficient space for compression.
	uLongf bufferSize = input.size() + input.size() / 10 + 12;
	output.resize(bufferSize);

	ret = ::compress2((Bytef*) output.data(), &bufferSize, (const Bytef*) input.data(),
		(uLong) input.size(), level); // attempt compression.

	if (ret != Z_OK)
	{
		return false;
	}

	// resize output to actual size.
	output.resize(bufferSize);

	return true;
}

bool decompressTransfer(const std::vector<char>& input, std::vector<char>& output)
{
	int ret;

	uLongf inputSize = input.size();
	uLongf bufferSize = inputSize;
	uLongf outputSize = bufferSize;
	output.resize(bufferSize);

	while ((ret = ::uncompress((Bytef*) output.data(), &bufferSize, (const Bytef*) input.data(),
		inputSize)) == Z_BUF_ERROR)
	{
		bufferSize += outputSize / 10;
		output.resize(bufferSize);
	}

	output.resize(bufferSize);

	if (ret != Z_OK)
	{
		return false;
	}

	return true;
}

}
