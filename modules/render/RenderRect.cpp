#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>
#include <panda/Renderer.h>
#include <QPainter>

namespace panda {

class RenderRect : public Renderer
{
public:
	PANDA_CLASS(RenderRect, Renderer)

	RenderRect(PandaDocument *parent)
		: Renderer(parent)
		, rect(initData(&rect, "rectangle", "Position and size of the rectangle"))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the outline of the rectangle"))
		, lineColor(initData(&lineColor, "lineColor", "Color of the outline of the rectangle"))
		, color(initData(&color, "color", "Color inside the rectangle"))
	{
		addInput(&rect);
		addInput(&lineWidth);
		addInput(&lineColor);
		addInput(&color);

		rect.getAccessor().push_back(QRectF(100, 100, 50, 50));
		lineColor.getAccessor().push_back(QColor(0,0,0));
		lineWidth.getAccessor().push_back(0.0);
	}

	void render(QPainter* painter)
	{
		painter->save();

		const QVector<QRectF>& listRect = rect.getValue();
		const QVector<QColor>& listLineColor = lineColor.getValue();
		const QVector<QColor>& listColor = color.getValue();
		const QVector<double>& listLineWidth = lineWidth.getValue();

		int nbRect = listRect.size();
		int nbLineColor = listLineColor.size();
		int nbColor = listColor.size();
		int nbLineWidth = listLineWidth.size();

		bool drawOutline = (nbLineWidth && nbLineColor);
		bool drawInside = (nbColor > 0);

		if(nbRect && (drawOutline || drawInside) )
		{
			painter->setBrush(Qt::NoBrush);
			painter->setPen(Qt::NoPen);

			if(nbLineColor < nbRect) nbLineColor = 1;
			if(nbLineWidth < nbRect) nbLineWidth = 1;
			if(nbColor < nbRect) nbColor = 1;

			for(int i=0; i<nbRect; ++i)
			{
				if(drawOutline)
				{
					QPen pen(listLineColor[i % nbLineColor]);
					pen.setWidthF(listLineWidth[i % nbLineWidth]);
					painter->setPen(pen);
				}

				if(drawInside)
					painter->setBrush(QBrush(listColor[i % nbColor]));

				painter->drawRect(listRect[i]);
			}
		}

		painter->restore();
	}

	void renderOpenGL()
	{
	}

protected:
	Data< QVector<QRectF> > rect;
	Data< QVector<double> > lineWidth;
	Data< QVector<QColor> > lineColor, color;
};

int RenderRectClass = RegisterObject<RenderRect>("Render/Rectangle").setDescription("Draw a plain rectangle");

} // namespace panda
