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
			m_output.getAccessor()->createTexture(buffer, size, 1);
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
