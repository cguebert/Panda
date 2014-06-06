#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>

#include <panda/helper/GradientCache.h>

#include <QOpenGLTexture>

namespace panda {

using types::Gradient;
using types::ImageWrapper;

class GradientTexture : public PandaObject
{
public:
	PANDA_CLASS(GradientTexture, PandaObject)

	GradientTexture(PandaDocument *doc)
		: PandaObject(doc)
		, m_gradient(initData(&m_gradient, "gradient", "Gradient used to create the texture"))
		, m_length(initData(&m_length, 256, "length", "Horizontal size of the texture created"))
		, m_output(initData(&m_output, "texture", "Texture created"))
	{
		addInput(&m_gradient);
		addInput(&m_length);

		addOutput(&m_output);
	}

	void update()
	{
		int size = m_length.getValue();
		if(size > 0)
		{
			auto buffer = helper::GradientCache::createBuffer(m_gradient.getValue(), size);
			bool create = false;
			if(!m_texture)
			{
				m_texture = QSharedPointer<QOpenGLTexture>(new QOpenGLTexture(QOpenGLTexture::Target2D));
				create = true;
			}
			else if(size != m_texture->width())
			{
				m_texture->destroy();
				create = true;
			}

			if(create)
			{
				m_texture->setSize(size, 1);
				m_texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
				m_texture->setMipLevels(m_texture->maximumMipLevels());
				m_texture->allocateStorage();
			}

			m_texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, buffer.constData());
			m_output.getAccessor()->setTexture(m_texture);
		}
		else
			m_output.getAccessor()->clear();

		cleanDirty();
	}

protected:
	Data<Gradient> m_gradient;
	Data<int> m_length;
	Data<ImageWrapper> m_output;

	QSharedPointer<QOpenGLTexture> m_texture;
};

int GradientTextureClass = RegisterObject<GradientTexture>("Generator/Gradient/Gradient texture").setDescription("Create a texture from a gradient");

} // namespace Panda
