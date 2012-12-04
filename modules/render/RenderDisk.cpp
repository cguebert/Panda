#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QPointF>
#include <QPainter>

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

		center.beginEdit()->append(QPointF(100, 100));
		center.endEdit();

		radius.beginEdit()->append(5.0);
		radius.endEdit();

		color.beginEdit()->append(QColor(0,0,0));
		color.endEdit();
	}

	void render(QPainter* painter)
	{
		painter->save();

		painter->setPen(Qt::NoPen);

		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbColor = listColor.size();

		if(nbCenter && nbRadius && nbColor)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbColor < nbCenter) nbColor = 1;

			for(int i=0; i<nbCenter; ++i)
			{
				painter->setBrush(QBrush(listColor[i % nbColor]));
				double valRadius = listRadius[i % nbRadius];
				painter->drawEllipse(listCenter[i], valRadius, valRadius);
			}
		}

		painter->restore();
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius;
	Data< QVector<QColor> > color;
};

int RenderDiskClass = RegisterObject("Render/Disk").setClass<RenderDisk>().setDescription("Draw a plain disk");

} // namespace panda
