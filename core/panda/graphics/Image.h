#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include <memory>
#include <string>
#include <vector>

namespace panda
{

namespace graphics
{

class Image
{
public:
	bool loadFromFile(const std::string& path);

protected:
	unsigned int m_textureId = 0;

	struct ImageData
	{
		std::vector<unsigned char> contents;
		int width = 0, height = 0;
		bool hasAlpha = false;
	};
	std::unique_ptr<ImageData> m_data;
};

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_IMAGE_H