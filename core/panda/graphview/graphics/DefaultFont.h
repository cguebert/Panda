#pragma once

unsigned int stb_decompress_length(const unsigned char *input);
unsigned int stb_decompress(unsigned char *output, unsigned char *input, unsigned int length);
const char*  GetDefaultCompressedFontDataTTFBase85();