#ifndef TYPES_IMAGEWRAPPER_H
#define TYPES_IMAGEWRAPPER_H

#include <QImage>
#include <QSharedPointer>
#include <QVector>

#include <panda/types/Color.h>

class QOpenGLTexture;
class QOpenGLFramebufferObject;

namespace panda
{

namespace types
{

class ImageWrapper
{
public:
	ImageWrapper();

	unsigned int getTextureId() const;
	const QImage& getImage() const;

	QSize size() const;
	int width() const;
	int height() const;

	void setImage(const QImage& img);
	void setFbo(QSharedPointer<QOpenGLFramebufferObject> fbo);
	void createTexture(QVector<types::Color> buffer, int width, int height);

	void clear();	/// Remove all sources
	bool isNull();	/// Does it have one valid source

	bool hasImageSource() const;
	bool hasTextureSource() const;
	bool hasFboSource() const;

	ImageWrapper& operator=(const ImageWrapper& rhs);

	QOpenGLFramebufferObject* getFbo() const; // will return null if image source

	bool operator==(const ImageWrapper& img) const;
	bool operator!=(const ImageWrapper& img) const;

private:
	void createImageFromBuffer();

	QImage m_image;
	QSharedPointer<QOpenGLTexture> m_texture;
	QSharedPointer<QOpenGLFramebufferObject> m_fbo;

	bool m_imageSource, m_textureSource, m_fboSource;
	int m_width, m_height;
	QVector<types::Color> m_buffer;
};

inline QSize ImageWrapper::size() const
{ return QSize(width(), height()); }

inline bool ImageWrapper::isNull()
{ return !(m_imageSource || m_textureSource || m_fboSource); }

inline bool ImageWrapper::hasImageSource() const
{ return m_imageSource; }

inline bool ImageWrapper::hasTextureSource() const
{ return m_textureSource; }

inline bool ImageWrapper::hasFboSource() const
{ return m_fboSource; }

inline QOpenGLFramebufferObject* ImageWrapper::getFbo() const
{ return m_fbo.data(); }

} // namespace types

} // namespace panda

#endif // TYPES_IMAGEWRAPPER_H
