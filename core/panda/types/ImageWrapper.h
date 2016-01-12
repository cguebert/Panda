#ifndef TYPES_IMAGEWRAPPER_H
#define TYPES_IMAGEWRAPPER_H

#include <panda/core.h>
#include <panda/graphics/Size.h>
#include <panda/types/Color.h>

#include <memory>
#include <vector>

class QOpenGLTexture;

namespace panda
{

namespace graphics
{
class Framebuffer;
class Image;
class Texture;
}

namespace types
{

class PANDA_CORE_API ImageWrapper
{
public:
	ImageWrapper();

	unsigned int getTextureId() const;
	const graphics::Image& getImage() const;

	graphics::Size size() const;
	int width() const;
	int height() const;

	void setImage(const graphics::Image& img);
	void setFbo(const graphics::Framebuffer& fbo);

	void createTexture(graphics::Size size, const std::vector<types::Color>& buffer);
	void createTexture(int width, int height, const std::vector<types::Color>& buffer);

	void clear(); /// Remove all sources
	bool isNull() const; /// Does it have one valid source

	bool hasImage() const; /// Return true if the source is an image
	bool hasTexture() const; /// Return true if the source is either a fbo or a texture

	ImageWrapper& operator=(const ImageWrapper& rhs);

	graphics::Framebuffer* getFbo() const; /// Will return null if image source

	bool operator==(const ImageWrapper& img) const;
	bool operator!=(const ImageWrapper& img) const;

private:
	void createImageFromBuffer();

	std::shared_ptr<graphics::Image> m_image;
	std::shared_ptr<graphics::Texture> m_texture;
	std::shared_ptr<graphics::Framebuffer> m_fbo;

	enum SourceType : unsigned char { NONE=0, IMAGE, TEXTURE, FBO };
	SourceType m_source;
	graphics::Size m_size;
	std::vector<types::Color> m_buffer;
};

inline graphics::Size ImageWrapper::size() const
{ return m_size; }

inline int ImageWrapper::width() const
{ return m_size.width(); }

inline int ImageWrapper::height() const
{ return m_size.height(); }

inline bool ImageWrapper::isNull() const
{ return m_source == NONE; }

inline bool ImageWrapper::hasImage() const
{ return m_source == IMAGE; }

inline bool ImageWrapper::hasTexture() const
{ return m_source == TEXTURE || m_source == FBO; }

inline graphics::Framebuffer* ImageWrapper::getFbo() const
{ return m_fbo.get(); }

inline void ImageWrapper::createTexture(int width, int height, const std::vector<types::Color>& buffer)
{ createTexture(graphics::Size(width, height), buffer); }

} // namespace types

} // namespace panda

#endif // TYPES_IMAGEWRAPPER_H
