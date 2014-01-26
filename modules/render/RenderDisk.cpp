#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Gradient.h>

#include <QPointF>
#include <QPainter>

using panda::types::Gradient;

namespace panda {

class RenderDisk : public Renderer
{
public:
	PANDA_CLASS(RenderDisk, Renderer)

	RenderDisk(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the disk"))
		, radius(initData(&radius, "radius", "Radius of the disk" ))
		, color(initData(&color, "color", "Color of the plain disk"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&color);

		center.getAccessor().push_back(QPointF(100, 100));
		radius.getAccessor().push_back(5.0);
		color.getAccessor().push_back(QColor(0,0,0));
	}

	void render(QPainter* painter)
	{
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbColor = listColor.size();

		if(nbCenter && nbRadius && nbColor)
		{
			painter->save();
			painter->setPen(Qt::NoPen);

			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;

			for(int i=0; i<nbCenter; ++i)
			{
				painter->setBrush(listColor[i % nbColor]);
				double valRadius = listRadius[i % nbRadius];
				painter->drawEllipse(listCenter[i], valRadius, valRadius);
			}

			painter->restore();
		}
	}

	void renderOpenGL()
	{
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius;
	Data< QVector<QColor> > color;
};

int RenderDiskClass = RegisterObject<RenderDisk>("Render/Disk").setDescription("Draw a plain disk");

//*************************************************************************//

class RenderDisk_Gradient : public Renderer
{
public:
	PANDA_CLASS(RenderDisk_Gradient, Renderer)

	RenderDisk_Gradient(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the disk"))
		, radius(initData(&radius, "radius", "Radius of the disk" ))
		, gradient(initData(&gradient, "gradient", "Gradient used to fill the disk"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&gradient);

		center.getAccessor().push_back(QPointF(100, 100));
		radius.getAccessor().push_back(5.0);

		Gradient grad;
		grad.add(0, QColor());
		grad.add(1, QColor(255, 255, 255));
		gradient.getAccessor().push_back(grad);
	}

	void render(QPainter* painter)
	{
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<Gradient>& listGradient = gradient.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbGradient = listGradient.size();

		if(nbCenter && nbRadius && nbGradient)
		{
			painter->save();
			painter->setPen(Qt::NoPen);

			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbGradient < nbCenter) nbGradient = 1;

			QRadialGradient grad;
			grad.setStops(listGradient[0].getStops());

			for(int i=0; i<nbCenter; ++i)
			{
				double valRadius = listRadius[i % nbRadius];
				if(nbGradient > 1 && i)
					grad.setStops(listGradient[i % nbGradient].getStops());
				grad.setCenter(listCenter[i]);
				grad.setFocalPoint(listCenter[i]);
				grad.setCenterRadius(valRadius);
				painter->setBrush(grad);
				painter->drawEllipse(listCenter[i], valRadius, valRadius);
			}

			painter->restore();
		}
	}

	void renderOpenGL()
	{
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius;
	Data< QVector<Gradient> > gradient;
};

int RenderDisk_GradientClass = RegisterObject<RenderDisk_Gradient>("Render/Gradient disk").setDescription("Draw a disk filled with a radial gradient");

} // namespace panda
