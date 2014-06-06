#include <panda/types/ImageWrapper.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

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
	else if(m_imageSource)
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
	if(m_fboSource && m_fbo)
		const_cast<ImageWrapper*>(this)->m_image = m_fbo->toImage();

	return m_image;
}

QSize ImageWrapper::size() const
{
	if(m_textureSource && m_texture)
		return QSize(m_texture->width(), m_texture->height());
	if(m_fboSource && m_fbo)
		return m_fbo->size();
	if(m_imageSource && !m_image.isNull())
		return m_image.size();
	return QSize();
}

int ImageWrapper::width() const
{
	if(m_textureSource && m_texture)
		return m_texture->width();
	if(m_imageSource)
		return m_image.width();
	if(m_fboSource && m_fbo)
		return m_fbo->width();
	return -1;
}

int ImageWrapper::height() const
{
	if(m_textureSource && m_texture)
		return m_texture->height();
	if(m_imageSource)
		return m_image.height();
	if(m_fboSource && m_fbo)
		return m_fbo->width();
	return -1;
}

void ImageWrapper::setImage(const QImage& img)
{
	m_image = img;
	m_texture.reset();
	m_fbo.reset();

	m_imageSource = true;
	m_textureSource = false;
	m_fboSource = false;
}

void ImageWrapper::setTexture(QSharedPointer<QOpenGLTexture> texture)
{
	m_image = QImage();
	m_texture = texture;
	m_fbo.reset();

	m_imageSource = false;
	m_textureSource = true;
	m_fboSource = false;
}

void ImageWrapper::setFbo(QSharedPointer<QOpenGLFramebufferObject> fbo)
{
	m_image = QImage();
	m_texture.reset();
	m_fbo = fbo;

	m_imageSource = false;
	m_textureSource = false;
	m_fboSource = true;
}

void ImageWrapper::clear()
{
	m_image = QImage();
	m_texture.reset();
	m_fbo.reset();

	m_imageSource = false;
	m_textureSource = false;
	m_fboSource = false;
}

bool ImageWrapper::hasImageSource() const
{
	return m_imageSource;
}

bool ImageWrapper::hasTextureSource() const
{
	return m_textureSource;
}

bool ImageWrapper::hasFboSource() const
{
	return m_fboSource;
}

ImageWrapper& ImageWrapper::operator=(const ImageWrapper& rhs)
{
	if(rhs.m_textureSource)
	{
		m_image = QImage();
		m_texture = rhs.m_texture;

		m_imageSource = false;
		m_textureSource = true;
	}
	else
	{
		m_image = rhs.getImage();
		m_texture.reset();

		m_imageSource = true;
		m_textureSource = false;
	}

	m_fbo.reset();
	m_fboSource = false;

	return *this;
}

QOpenGLFramebufferObject* ImageWrapper::getFbo() const
{
	return m_fbo.data();
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

template<> QString DataTrait<ImageWrapper>::valueTypeName() { return "image"; }

template<> void DataTrait<ImageWrapper>::writeValue(QDomDocument&, QDomElement&, const ImageWrapper&) {}
template<> void DataTrait<ImageWrapper>::readValue(QDomElement&, ImageWrapper&) {}

template<> bool DataTrait<ImageWrapper>::isDisplayed() { return false; }
template<> bool DataTrait<ImageWrapper>::isPersistent() { return false; }

template class Data< ImageWrapper >;
template class Data< QVector<ImageWrapper> >;

int imageDataClass = RegisterData< ImageWrapper >();
int imageVectorDataClass = RegisterData< QVector<ImageWrapper> >();

} // namespace types

} // namespace panda
