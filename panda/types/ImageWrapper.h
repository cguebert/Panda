#ifndef TYPES_IMAGEWRAPPER_H
#define TYPES_IMAGEWRAPPER_H

#include <QImage>
#include <QSharedPointer>

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
	void setTexture(QSharedPointer<QOpenGLTexture> texture);
	void setFbo(QSharedPointer<QOpenGLFramebufferObject> fbo);
	void clear();	// Remove all sources

	bool hasImageSource() const;
	bool hasTextureSource() const;
	bool hasFboSource() const;

	ImageWrapper& operator=(const ImageWrapper& rhs);

	QOpenGLFramebufferObject* getFbo() const; // will return null if image source

	bool operator==(const ImageWrapper& img) const;
	bool operator!=(const ImageWrapper& img) const;

private:
	QImage m_image;
	QSharedPointer<QOpenGLTexture> m_texture;
	QSharedPointer<QOpenGLFramebufferObject> m_fbo;

	bool m_imageSource, m_textureSource, m_fboSource;
};

} // namespace types

} // namespace panda

#endif // TYPES_IMAGEWRAPPER_H
