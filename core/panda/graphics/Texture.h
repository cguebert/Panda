#ifndef GRAPHICS_TEXTURE_H
#define GRAPHICS_TEXTURE_H

#include <panda/graphics/Size.h>

#include <memory>
#include <vector>

namespace panda
{

namespace graphics
{

// To destroy the texture object only when every copy is destroyed
struct TextureData;

class Image;

class PANDA_CORE_API Texture
{
public:
	Texture() {}
	Texture(const Image& img);
	Texture(Size size, const float* data); // data is in GL_RGBA32F format
	Texture(Size size, const unsigned char* data); // data is in GL_RGBA format

	explicit operator bool() const; // Returns true if an OpenGL texture has been created for this object

	void update(const float* data);

	Size size() const;
	unsigned int id() const;

	enum class WrapMode { Repeat, MirroredRepeat, ClampToEdge };
	void setWrapMode(WrapMode mode);

private:
	std::shared_ptr<TextureData> m_data;
};

inline Texture::operator bool() const
{ return m_data != nullptr; }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_TEXTURE_H
