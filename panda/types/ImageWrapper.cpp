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
	, m_fboSource(false)
{}

GLuint ImageWrapper::getTexture() const
{
	if(m_fboSource && m_fbo)
		return m_fbo->texture();
	else if(m_imageSource)
	{
		if(!m_texture)
			const_cast<ImageWrapper*>(this)->m_texture
				= QSharedPointer<QOpenGLTexture>(new QOpenGLTexture(m_image));

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
	if(m_fboSource && m_fbo)
		return m_fbo->size();
	if(m_imageSource && !m_image.isNull())
		return m_image.size();
	return QSize();
}

void ImageWrapper::setImage(const QImage& img)
{
	m_image = img;
	m_imageSource = true;
	m_fboSource = false;
	m_texture.reset();
}

void ImageWrapper::setFbo(QSharedPointer<QOpenGLFramebufferObject> fbo)
{
	m_fbo = fbo;
	m_fboSource = true;
	m_imageSource = false;
}

ImageWrapper& ImageWrapper::operator=(const ImageWrapper& rhs)
{
	m_image = rhs.getImage();
	return *this;
}

//***************************************************************//

template<> QString DataTrait<ImageWrapper>::valueTypeName() { return "image"; }

template<> bool DataTrait<ImageWrapper>::isDisplayed() { return false; }
template<> bool DataTrait<ImageWrapper>::isPersistent() { return false; }

template class Data< ImageWrapper >;
template class Data< QVector<ImageWrapper> >;

int imageDataClass = RegisterData< ImageWrapper >();
int imageVectorDataClass = RegisterData< QVector<ImageWrapper> >();

} // namespace types

} // namespace panda
