#ifndef HELPER_FONT_H
#define HELPER_FONT_H

#include <panda/core.h>

#include <string>

namespace panda
{

namespace helper
{

class PANDA_CORE_API Font
{
public:
	Font() = default;
	Font(const std::string& text);

	std::string toString() const;

	std::string name = "Times New Roman"; // Name of the font
	std::string path = "times.ttf"; // Real path to the font file
	int faceIndex = 0; // Index of the face in the file
	int pointSize = 12;
	int weight = 50; // 0 - 99
	bool italic = false;
};

} // namespace helper

} // namespace panda

#endif // HELPER_FONT_H
