#include <panda/graphics/Texture.h>

#include <GL/glew.h>

namespace panda
{

namespace graphics
{

class TextureId
{
public:
	TextureId(unsigned int id) : m_id(id) {}
	~TextureId() { glDeleteTextures(1, &m_id); }
	unsigned int id() { return m_id; }

private:
	unsigned int m_id = 0;
};

//****************************************************************************//

Texture Texture::fromImage(const Image& img)
{
	return Texture();
}

unsigned int Texture::id() const
{
	if (m_id)
		return m_id->id();
	return 0;
}

} // namespace graphics

} // namespace panda

