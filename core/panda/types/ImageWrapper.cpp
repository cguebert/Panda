#include <panda/types/ImageWrapper.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>

namespace panda
{

namespace types
{

ImageWrapper::ImageWrapper()
	: m_imageSource(false)
	, m_textureSource(false)
	, m_fboSource(false)
{}

GLuint ImageWrapper::getTextureId() const
{
	if(m_textureSource && m_texture)
		return m_texture->textureId();
	if(m_fboSource && m_fbo)
		return m_fbo->texture();
	if(m_imageSource)
	{
		if(!m_texture)
			const_cast<ImageWrapper*>(this)->m_texture
				= QSharedPointer<QOpenGLTexture>(new QOpenGLTexture(m_image.mirrored()));

		return m_texture->textureId();
	}
	return 0;
}

const QImage& ImageWrapper::getImage() const
{
	if(m_fboSource && m_fbo && m_image.isNull())
		const_cast<ImageWrapper*>(this)->m_image = m_fbo->toImage();
	else if(m_textureSource && !m_buffer.isEmpty() && m_image.isNull())
		const_cast<ImageWrapper*>(this)->createImageFromBuffer();

	return m_image;
}

int ImageWrapper::width() const
{
	if(m_fboSource && m_fbo)
		return m_fbo->width();
	return m_width;
}

int ImageWrapper::height() const
{
	if(m_fboSource && m_fbo)
		return m_fbo->height();
	return m_height;
}

void ImageWrapper::setImage(const QImage& img)
{
	m_image = img;
	m_texture.reset();
	m_fbo.reset();

	m_imageSource = true;
	m_textureSource = false;
	m_fboSource = false;

	m_width = m_image.width();
	m_height = m_image.height();
	m_buffer.clear();
}

void ImageWrapper::setFbo(QSharedPointer<QOpenGLFramebufferObject> fbo)
{
	m_image = QImage();
	m_texture.reset();
	m_fbo = fbo;

	m_imageSource = false;
	m_textureSource = false;
	m_fboSource = true;

	m_buffer.clear();
}

void ImageWrapper::createTexture(QVector<types::Color> buffer, int width, int height)
{
	m_buffer = buffer;
	m_width = width;
	m_height = height;

	m_image = QImage();
	m_fbo.reset();

	m_texture = QSharedPointer<QOpenGLTexture>(new QOpenGLTexture(QOpenGLTexture::Target2D));
	m_texture->setSize(width, height);
	m_texture->setFormat(QOpenGLTexture::RGBA32F);
	m_texture->setMipLevels(m_texture->maximumMipLevels());
	m_texture->allocateStorage();

	m_texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, buffer.constData());

	m_imageSource = false;
	m_textureSource = true;
	m_fboSource = false;
}

void ImageWrapper::createImageFromBuffer()
{
	m_image = QImage(m_width, m_height, QImage::Format_ARGB32);
	int nbPixels = m_width * m_height;
	QVector<uchar> imgBuffer(nbPixels * 4);
	for(int i=0; i<nbPixels; ++i)
	{
		imgBuffer[i*4  ] = m_buffer[i].b * 255;
		imgBuffer[i*4+1] = m_buffer[i].g * 255;
		imgBuffer[i*4+2] = m_buffer[i].r * 255;
		imgBuffer[i*4+3] = m_buffer[i].a * 255;
	}
	memcpy(m_image.bits(), imgBuffer.constData(), nbPixels * 4);
}

void ImageWrapper::clear()
{
	m_image = QImage();
	m_texture.reset();
	m_fbo.reset();

	m_imageSource = false;
	m_textureSource = false;
	m_fboSource = false;

	m_buffer.clear();
	m_width = -1;
	m_height = -1;
}

ImageWrapper& ImageWrapper::operator=(const ImageWrapper& rhs)
{
	m_image = rhs.getImage();
	m_texture.reset();
	m_fbo.reset();

	m_imageSource = true;
	m_textureSource = false;
	m_fboSource = false;

	m_buffer.clear();
	m_width = m_image.width();
	m_height = m_image.height();

	return *this;
}

bool ImageWrapper::operator==(const ImageWrapper& img) const
{
	if(m_textureSource != img.m_textureSource)
		return false;

	if(m_textureSource && img.m_textureSource && m_texture != img.m_texture)
		return false;

	return m_image == img.m_image;
}

bool ImageWrapper::operator!=(const ImageWrapper& img) const
{
	return !(*this == img);
}

//****************************************************************************//

template<> PANDA_CORE_API QString DataTrait<ImageWrapper>::valueTypeName() { return "image"; }

template<> PANDA_CORE_API void DataTrait<ImageWrapper>::writeValue(QDomDocument&, QDomElement&, const ImageWrapper&) {}
template<> PANDA_CORE_API void DataTrait<ImageWrapper>::readValue(QDomElement&, ImageWrapper&) {}

template<> PANDA_CORE_API bool DataTrait<ImageWrapper>::isDisplayed() { return false; }
template<> PANDA_CORE_API bool DataTrait<ImageWrapper>::isPersistent() { return false; }

template class Data< ImageWrapper >;
template class Data< QVector<ImageWrapper> >;

int imageDataClass = RegisterData< ImageWrapper >();
int imageVectorDataClass = RegisterData< QVector<ImageWrapper> >();

} // namespace types

} // namespace panda
