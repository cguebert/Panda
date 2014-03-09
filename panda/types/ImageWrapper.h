#ifndef IMAGEWRAPPER_H
#define IMAGEWRAPPER_H

#include <QImage>
#include <QSharedPointer>
#include <QtGui/qopengl.h>

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

	GLuint getTexture() const;
	const QImage &getImage() const;

	void setImage(const QImage& img);
	void setFbo(QSharedPointer<QOpenGLFramebufferObject> fbo);

	ImageWrapper& operator=(const ImageWrapper& rhs);

private:
	QImage m_image;
	QSharedPointer<QOpenGLTexture> m_texture;
	QSharedPointer<QOpenGLFramebufferObject> m_fbo;

	bool m_imageSource, m_fboSource;
};

} // namespace types

} // namespace panda

#endif // IMAGEWRAPPER_H
