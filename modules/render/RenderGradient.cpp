#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Gradient.h>
#include <panda/helper/GradientCache.h>

#include <QPointF>

namespace panda {

using types::Gradient;

class RenderGradient_Horizontal : public Renderer
{
public:
	PANDA_CLASS(RenderGradient_Horizontal, Renderer)

	RenderGradient_Horizontal(PandaDocument *parent)
		: Renderer(parent)
		, gradient(initData(&gradient, "gradient", "Gradient to paint on the screen"))
	{
		addInput(&gradient);
	}

	void render()
	{
		const Gradient& grad = gradient.getValue();

		QSize size = parentDocument->getRenderSize();
		GLuint texture = helper::GradientCache::getInstance()->getTexture(grad, size.width());
		if(texture == -1)
			return;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		PReal verts[8], texCoords[8];

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_PREAL, 0, verts);
		verts[0*2+0] = size.width(); verts[0*2+1] = 0;
		verts[1*2+0] = 0; verts[1*2+1] = 0;
		verts[2*2+0] = size.width(); verts[2*2+1] = size.height();
		verts[3*2+0] = 0; verts[3*2+1] = size.height();

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_PREAL, 0, texCoords);
		texCoords[0*2+0] = 1; texCoords[0*2+1] = 0;
		texCoords[1*2+0] = 0; texCoords[1*2+1] = 0;
		texCoords[2*2+0] = 1; texCoords[2*2+1] = 0;
		texCoords[3*2+0] = 0; texCoords[3*2+1] = 0;

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

protected:
	Data<Gradient> gradient;
};

int RenderGradient_HorizontalClass = RegisterObject<RenderGradient_Horizontal>("Render/Horizontal Gradient").setDescription("Draw a horizontal gradient taking the full screen");

class RenderGradient_Vertical : public Renderer
{
public:
	PANDA_CLASS(RenderGradient_Vertical, Renderer)

	RenderGradient_Vertical(PandaDocument *parent)
		: Renderer(parent)
		, gradient(initData(&gradient, "gradient", "Gradient to paint on the screen"))
	{
		addInput(&gradient);
	}

	void render()
	{
		const Gradient& grad = gradient.getValue();

		QSize size = parentDocument->getRenderSize();
		GLuint texture = helper::GradientCache::getInstance()->getTexture(grad, size.height());
		if(texture == -1)
			return;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		PReal verts[8], texCoords[8];

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_PREAL, 0, verts);
		verts[0*2+0] = size.width(); verts[0*2+1] = 0;
		verts[1*2+0] = 0; verts[1*2+1] = 0;
		verts[2*2+0] = size.width(); verts[2*2+1] = size.height();
		verts[3*2+0] = 0; verts[3*2+1] = size.height();

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_PREAL, 0, texCoords);
		texCoords[0*2+0] = 0; texCoords[0*2+1] = 0;
		texCoords[1*2+0] = 0; texCoords[1*2+1] = 0;
		texCoords[2*2+0] = 1; texCoords[2*2+1] = 0;
		texCoords[3*2+0] = 1; texCoords[3*2+1] = 0;

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

protected:
	Data<Gradient> gradient;
};

int RenderGradient_VerticalClass = RegisterObject<RenderGradient_Vertical>("Render/Vertical Gradient").setDescription("Draw a vertical gradient taking the full screen");


} // namespace panda
