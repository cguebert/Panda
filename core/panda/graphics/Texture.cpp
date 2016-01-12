#include <panda/graphics/Texture.h>
#include <panda/graphics/Image.h>

#include <GL/glew.h>

#include <algorithm>
#include <cmath>

namespace
{

inline int mipLevelSize(int mipLevel, int baseSize)
{
    return std::max(1, baseSize >> mipLevel);
}

GLuint createTexture(panda::graphics::Size size, GLenum format, GLenum type, const void* data)
{
	const int w = size.width(), h = size.height();
	GLuint id = 0;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	int nbLevels = 1 + static_cast<int>(std::floor(std::log2(std::max(w, h))));
	if (GLEW_ARB_texture_storage || GLEW_VERSION_4_2)
		glTextureStorage2D(id, nbLevels, GL_RGBA32F, w, h);
	else
	{
		for (int level = 0; level < nbLevels; ++level)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mipLevelSize(level, w), mipLevelSize(level, h), 0, format, type, data);
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, type, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	return id;
}

}

namespace panda
{

namespace graphics
{

struct TextureData
{
	~TextureData() 
	{ glDeleteTextures(1, &id); }

	Size size;
	unsigned int id = 0;
};

//****************************************************************************//

Texture::Texture(const Image& img)
{
	GLuint id = createTexture(img.size(), GL_RGBA, GL_UNSIGNED_BYTE, img.data());

	if (id)
	{
		m_data = std::make_shared<TextureData>();
		m_data->id = id;
		m_data->size = img.size();
	}
}

Texture::Texture(Size size, const float* data)
	: m_data(std::make_shared<TextureData>())
{
	GLuint id = createTexture(size, GL_RGBA, GL_FLOAT, data);

	if (id)
	{
		m_data = std::make_shared<TextureData>();
		m_data->id = id;
		m_data->size = size;
	}
}

void Texture::update(const float* data)
{
	auto texId = id();
	if (!texId)
		return;

	glBindTexture(GL_TEXTURE_2D, texId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_data->size.width(), m_data->size.height(), GL_RGBA, GL_FLOAT, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

Size Texture::size() const
{
	if (m_data)
		return m_data->size;
	return Size();
}

unsigned int Texture::id() const
{
	if (m_data)
		return m_data->id;
	return 0;
}

void Texture::setWrapMode(WrapMode mode)
{
	auto texId = id();
	if (!texId)
		return;

	GLint modeVal;
	switch (mode)
	{
	case WrapMode::Repeat: modeVal = GL_REPEAT; break;
	case WrapMode::MirroredRepeat: modeVal = GL_MIRRORED_REPEAT; break;
	case WrapMode::ClampToEdge: modeVal = GL_CLAMP_TO_EDGE; break;
	}

	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, modeVal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, modeVal);
}

} // namespace graphics

} // namespace panda

