#include <panda/types/ImageWrapper.h>

#include <panda/data/DataFactory.h>

#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Image.h>
#include <panda/graphics/Texture.h>

#include <cassert>

namespace panda
{

namespace types
{

using graphics::Framebuffer;
using graphics::Image;
using graphics::Size;
using graphics::Texture;

ImageWrapper::ImageWrapper()
	: m_source(NONE)
{}

unsigned int ImageWrapper::getTextureId() const
{
	if(m_source == TEXTURE && m_texture)
		return m_texture->id();
	if(m_source == FBO && m_fbo)
		return m_fbo->texture();
	if(m_source == IMAGE && m_image)
	{
		if (!m_texture)
			const_cast<ImageWrapper*>(this)->m_texture = std::make_shared<Texture>(*m_image);

		return m_texture->id();
	}
	return 0;
}

const Image& ImageWrapper::getImage() const
{
	auto nonConst = const_cast<ImageWrapper*>(this);
	if (m_source == FBO && m_fbo)
	{
		if (m_image)
			m_fbo->toImage(*nonConst->m_image);
		else
			nonConst->m_image = std::make_shared<Image>(m_fbo->toImage());
	}
	else if(m_source == TEXTURE && !m_buffer.empty() && !m_image)
		nonConst->createImageFromBuffer();

	if(!m_image)
		nonConst->m_image = std::make_shared<Image>();

	return *m_image;
}

void ImageWrapper::setImage(const Image& img)
{
	clear();
	m_image = std::make_shared<Image>(img);
	m_source = IMAGE;
	m_size = img.size();
}

void ImageWrapper::setFbo(const Framebuffer& fbo)
{
	clear();
	m_fbo = std::make_shared<Framebuffer>(fbo);
	m_source = FBO;
	m_size = fbo.size();
}

void ImageWrapper::createTexture(Size size, const std::vector<types::Color>& buffer)
{
	m_buffer = buffer;
	m_size = size;

	m_image.reset();
	m_fbo.reset();

	assert(buffer.size() == size.width() * size.height());

	if (!m_texture || m_texture->size() != size)
		m_texture = std::make_shared<Texture>(m_size, buffer.front().data());
	else
		m_texture->update(buffer.front().data());

	m_source = TEXTURE;
}

void ImageWrapper::createImageFromBuffer()
{
	int nbPixels = m_size.width() * m_size.height();
	std::vector<unsigned char> imgBuffer(nbPixels * 4);
	for(int i=0; i<nbPixels; ++i)
	{
		imgBuffer[i*4  ] = static_cast<unsigned char>(m_buffer[i].b * 255);
		imgBuffer[i*4+1] = static_cast<unsigned char>(m_buffer[i].g * 255);
		imgBuffer[i*4+2] = static_cast<unsigned char>(m_buffer[i].r * 255);
		imgBuffer[i*4+3] = static_cast<unsigned char>(m_buffer[i].a * 255);
	}
	m_image = std::make_shared<Image>(m_size, imgBuffer);
}

void ImageWrapper::clear()
{
	m_image.reset();
	m_texture.reset();
	m_fbo.reset();

	m_source = NONE;

	m_buffer.clear();
	m_size = Size();
}

ImageWrapper& ImageWrapper::operator=(const ImageWrapper& rhs)
{
	clear();

	if (rhs.m_source == NONE)
	{
		return *this;
	}
	else if(rhs.getFbo())
	{ // Copy the FBO
		m_source = FBO;
		Framebuffer* rhsFBO = rhs.getFbo();
		m_fbo = std::make_shared<Framebuffer>(rhsFBO->size());
		Framebuffer::blitFramebuffer(*m_fbo, *rhsFBO);
		m_size = rhs.size();
	}
	else
	{ // Create an image
		m_source = IMAGE;
		m_image = std::make_shared<Image>(rhs.getImage().clone());
		m_size = rhs.size();
	}

	return *this;
}

bool ImageWrapper::operator==(const ImageWrapper& img) const
{
	if(m_source != img.m_source)
		return false;

	if(m_source == TEXTURE && m_texture != img.m_texture)
		return false;

	if(m_source == FBO && m_fbo != img.m_fbo)
		return false;

	return m_image == img.m_image;
}

bool ImageWrapper::operator!=(const ImageWrapper& img) const
{
	return !(*this == img);
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<ImageWrapper>::valueTypeName() { return "image"; }

template<> PANDA_CORE_API void DataTrait<ImageWrapper>::writeValue(XmlElement&, const ImageWrapper&) {}
template<> PANDA_CORE_API void DataTrait<ImageWrapper>::readValue(XmlElement&, ImageWrapper&) {}

template<> PANDA_CORE_API bool DataTrait<ImageWrapper>::isDisplayed() { return false; }
template<> PANDA_CORE_API bool DataTrait<ImageWrapper>::isPersistent() { return false; }

template class Data< ImageWrapper >;
template class Data< std::vector<ImageWrapper> >;

int imageDataClass = RegisterData< ImageWrapper >();
int imageVectorDataClass = RegisterData< std::vector<ImageWrapper> >();

} // namespace types

} // namespace panda
