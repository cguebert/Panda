#ifndef GRAPHICS_TEXTURE_H
#define GRAPHICS_TEXTURE_H

#include <memory>
#include <string>
#include <vector>

namespace panda
{

namespace graphics
{

// To destroy the texture object only when every copy is destroyed
class TextureId;

class Image;

class Texture
{
public:
	static Texture fromImage(const Image& img);
	unsigned int id() const;

protected:
	std::shared_ptr<TextureId> m_id;
};

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_TEXTURE_H
