#ifndef ZLIB_UTIL_HPP
#define ZLIB_UTIL_HPP

#include <vector>

namespace zu
{
/*
 enum retCode {
 zOk = 0,
 zStreamEnd = 1,
 zNeedDict = 2,
 zErrNo = -1,
 zStreamError = -2,
 zDataError = -3,
 zMemError = -4,
 zBufError = -5,
 zVersionError = -6,

 zHeaderError = -101
 };
 */
bool compress(std::vector<char> & data, int level = 6);
bool decompress(std::vector<char> & data);

bool compressTransfer(const std::vector<char> & input, std::vector<char> & output, int level = 6);
bool decompressTransfer(const std::vector<char> & input, std::vector<char> & output);
}

#endif
