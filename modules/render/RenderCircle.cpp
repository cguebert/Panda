#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QPointF>
#include <QPainter>

namespace panda {

class RenderCircle : public Renderer
{
public:
	PANDA_CLASS(RenderCircle, Renderer)

	RenderCircle(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the circle"))
		, radius(initData(&radius, "radius", "Radius of the circle" ))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the circle's line"))
		, lineColor(initData(&lineColor, "lineColor", "Color of the circle"))
		, color(initData(&color, "color", "Color inside of the circle"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&lineWidth);
		addInput(&lineColor);
		addInput(&color);

		center.beginEdit()->append(QPointF(100, 100));
		center.endEdit();

		radius.beginEdit()->append(5.0);
		radius.endEdit();

		lineWidth.beginEdit()->append(0.0);
		lineWidth.endEdit();

		lineColor.beginEdit()->append(QColor(0,0,0));
		lineColor.endEdit();
	}

	void render(QPainter* painter)
	{
		const QVector<QPointF>& listCenter = center.getValue();
		const QVector<double>& listRadius = radius.getValue();
		const QVector<double>& listWidth = lineWidth.getValue();
		const QVector<QColor>& listLineColor = lineColor.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbWidth = listWidth.size();
		int nbLineColor = listLineColor.size();
		int nbColor = listColor.size();

		bool drawOutline = (nbWidth && nbLineColor);
		bool drawInside = (nbColor > 0);

		if(nbCenter && nbRadius && ( drawOutline || drawInside) )
		{
			painter->save();
			painter->setBrush(Qt::NoBrush);
			painter->setPen(Qt::NoPen);

			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbWidth < nbCenter) nbWidth = 1;
			if(nbLineColor < nbCenter) nbLineColor = 1;
			if(nbColor < nbCenter) nbColor = 1;

			for(int i=0; i<nbCenter; ++i)
			{
				if(drawOutline)
				{
					QPen pen(listLineColor[i % nbLineColor]);
					pen.setWidthF(listWidth[i % nbWidth]);
					painter->setPen(pen);
				}
				if(drawInside)
					painter->setBrush(QBrush(listColor[i % nbColor]));

				double valRadius = listRadius[i % nbRadius];
				painter->drawEllipse(listCenter[i], valRadius, valRadius);
			}

			painter->restore();
		}
	}

protected:
	Data< QVector<QPointF> > center;
	Data< QVector<double> > radius, lineWidth;
	Data< QVector<QColor> > lineColor, color;
};

int RenderCircleClass = RegisterObject("Render/Circle").setClass<RenderCircle>().setDescription("Draw a circle");

} // namespace panda
