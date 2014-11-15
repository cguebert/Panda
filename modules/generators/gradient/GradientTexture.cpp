#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>

#include <panda/helper/GradientCache.h>

#include <QOpenGLTexture>

namespace panda {

using types::Color;
using types::Gradient;
using types::ImageWrapper;
using types::Point;

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

int GradientTextureClass = RegisterObject<GradientTexture>("Generator/Gradient/Gradient texture").setDescription("Create a 1d texture from a gradient");

//****************************************************************************//

class GradientTexture2D : public PandaObject
{
public:
	PANDA_CLASS(GradientTexture2D, PandaObject)

	GradientTexture2D(PandaDocument *doc)
		: PandaObject(doc)
		, m_horizontalGradient(initData(&m_horizontalGradient, "horizontal", "Gradient used in the horizontal direction"))
		, m_verticalGradient(initData(&m_verticalGradient, "vertical", "Gradient used in the vertical direction"))
		, m_size(initData(&m_size, Point(256, 256), "size", "Dimensions of the texture created"))
		, m_output(initData(&m_output, "texture", "Texture created"))
	{
		addInput(&m_horizontalGradient);
		addInput(&m_verticalGradient);
		addInput(&m_size);

		addOutput(&m_output);
	}

	void update()
	{
		Point size = m_size.getValue();
		int width = (int)size.x, height = (int)size.y;
		if(width * height > 0)
		{
			auto bufHor = helper::GradientCache::createBuffer(m_horizontalGradient.getValue(), width);
			auto bufVer = helper::GradientCache::createBuffer(m_verticalGradient.getValue(), height);

			QVector<Color> buffer(width * height);
			for(int y=0; y<height; ++y)
				for(int x=0; x<width; ++x)
					buffer[y*width+x] = bufHor[x] * bufVer[y];

			m_output.getAccessor()->createTexture(buffer, width, height);
		}
		else
			m_output.getAccessor()->clear();

		cleanDirty();
	}

protected:
	Data<Gradient> m_horizontalGradient, m_verticalGradient;
	Data<Point> m_size;
	Data<ImageWrapper> m_output;

	QSharedPointer<QOpenGLTexture> m_texture;
};

int GradientTexture2DClass = RegisterObject<GradientTexture2D>("Generator/Gradient/2 gradients texture").setDescription("Create a 2d texture from the multiplication of 2 gradients");


} // namespace Panda
