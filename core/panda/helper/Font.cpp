#include <panda/helper/Font.h>

#include <sstream>

namespace panda
{

namespace helper
{

Font::Font(const std::string& text)
{
	if (text.empty())
	{
		*this = Font();
		return;
	}

	const char delim = ',';
	std::istringstream in(text);
	std::getline(in, name, delim);
	std::getline(in, path, delim);

	in >> faceIndex;	in.get();
	in >> pointSize;	in.get();
	in >> weight;		in.get();
	in >> italic;
}

std::string Font::toString() const
{
	const char delim = ',';
	std::string text = name + delim
		+ path + delim
		+ std::to_string(faceIndex) + delim
		+ std::to_string(pointSize) + delim
		+ std::to_string(weight) + delim
		+ (italic ? "1" : "0");
	return text;
}

} // namespace helper

} // namespace panda
