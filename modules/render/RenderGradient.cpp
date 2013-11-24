#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Gradient.h>

#include <QPointF>
#include <QPainter>

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

	void render(QPainter* painter)
	{
		painter->save();

		const Gradient& grad = gradient.getValue();
		QSize size = parentDocument->getRenderSize();
		for(int x=0; x<size.width(); ++x)
		{
			double p = (double)x / size.width();
			QColor c = grad.get(p);
			painter->fillRect(x, 0, 1, size.height(), c);
		}

		painter->restore();
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

	void render(QPainter* painter)
	{
		painter->save();

		const Gradient& grad = gradient.getValue();
		QSize size = parentDocument->getRenderSize();
		for(int y=0; y<size.height(); ++y)
		{
			double p = (double)y / size.height();
			QColor c = grad.get(p);
			painter->fillRect(0, y, size.width(), 1, c);
		}

		painter->restore();
	}

protected:
	Data<Gradient> gradient;
};

int RenderGradient_VerticalClass = RegisterObject<RenderGradient_Vertical>("Render/Vertical Gradient").setDescription("Draw a vertical gradient taking the full screen");


} // namespace panda
