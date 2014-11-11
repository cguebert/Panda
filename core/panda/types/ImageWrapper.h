#ifndef TYPES_IMAGEWRAPPER_H
#define TYPES_IMAGEWRAPPER_H

#include <panda/core.h>
#include <panda/types/Color.h>

#include <QImage>
#include <QSharedPointer>
#include <QVector>

class QOpenGLTexture;
class QOpenGLFramebufferObject;

namespace panda
{

namespace types
{

class PANDA_CORE_API ImageWrapper
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

	void clear(); /// Remove all sources
	bool isNull() const; /// Does it have one valid source

	bool hasImage() const; /// Return true if the source is an image
	bool hasTexture() const; /// Return true if the source is either a fbo or a texture

	ImageWrapper& operator=(const ImageWrapper& rhs);

	QOpenGLFramebufferObject* getFbo() const; /// Will return null if image source

	bool operator==(const ImageWrapper& img) const;
	bool operator!=(const ImageWrapper& img) const;

private:
	void createImageFromBuffer();

	QImage m_image;
	QSharedPointer<QOpenGLTexture> m_texture;
	QSharedPointer<QOpenGLFramebufferObject> m_fbo;

	enum SourceType : unsigned char { NONE=0, IMAGE, TEXTURE, FBO };
	SourceType m_source;
	int m_width, m_height;
	QVector<types::Color> m_buffer;
};

inline QSize ImageWrapper::size() const
{ return QSize(width(), height()); }

inline bool ImageWrapper::isNull() const
{ return m_source == NONE; }

inline bool ImageWrapper::hasImage() const
{ return m_source == IMAGE; }

inline bool ImageWrapper::hasTexture() const
{ return m_source == TEXTURE || m_source == FBO; }

inline QOpenGLFramebufferObject* ImageWrapper::getFbo() const
{ return m_fbo.data(); }

} // namespace types

} // namespace panda

#endif // TYPES_IMAGEWRAPPER_H
