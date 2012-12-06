#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>
#include <panda/Renderer.h>
#include <QPainter>

namespace panda {

class RenderLine : public Renderer
{
public:
	PANDA_CLASS(RenderLine, Renderer)

	RenderLine(PandaDocument *parent)
		: Renderer(parent)
		, inputA(initData(&inputA, "point 1", "Start of the line"))
		, inputB(initData(&inputB, "point 2", "Start of the line"))
		, width(initData(&width, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&width);
		addInput(&color);

		width.beginEdit()->append(0.0);
		width.endEdit();

		color.beginEdit()->append(QColor());
		color.endEdit();
	}

	void render(QPainter* painter)
	{
		const QVector<QPointF>& valA = inputA.getValue();
		const QVector<QPointF>& valB = inputB.getValue();
		const QVector<double>& listWidth = width.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbPts = qMin(valA.size(), valB.size());
		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		if(nbPts && nbWidth && nbColor)
		{
			if(nbWidth < nbPts) nbWidth = 1;
			if(nbColor < nbPts) nbColor = 1;

			painter->save();
			painter->setBrush(Qt::NoBrush);

			for(int i=0; i<nbPts; ++i)
			{
				QPen pen(listColor[i % nbColor]);
				pen.setWidthF(listWidth[i % nbWidth]);
				pen.setCapStyle(Qt::RoundCap);
				painter->setPen(pen);

				painter->drawLine(valA[i], valB[i]);
			}

			painter->restore();
		}
	}

protected:
	Data< QVector<QPointF> > inputA, inputB;
	Data< QVector<double> > width;
	Data< QVector<QColor> > color;
};

int RenderLineClass = RegisterObject("Render/Line").setClass<RenderLine>().setDescription("Draw a line between 2 points");

//*************************************************************************//

class RenderConnectedLines : public Renderer
{
public:
	PANDA_CLASS(RenderConnectedLines, Renderer)

	RenderConnectedLines(PandaDocument *parent)
		: Renderer(parent)
		, input(initData(&input, "points", "Vertices of the connected lines"))
		, width(initData(&width, 0.0, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&input);
		addInput(&width);
		addInput(&color);
	}

	void render(QPainter* painter)
	{
		painter->save();

		const QVector<QPointF>& points = input.getValue();
		int nb = points.size();

		if(nb)
		{
			QPen pen;
			pen.setWidthF(width.getValue());
			pen.setColor(color.getValue());
			pen.setCapStyle(Qt::RoundCap);
			painter->setBrush(Qt::NoBrush);
			painter->setPen(pen);

			QPainterPath path;
			path.moveTo(points[0]);
			for(int i=1; i<nb; ++i)
				path.lineTo(points[i]);

			painter->drawPath(path);
		}
		painter->restore();
	}

protected:
	Data< QVector<QPointF> > input;
	Data<double> width;
	Data<QColor> color;
};

int RenderConnectedLinesClass = RegisterObject("Render/Connected lines").setClass<RenderConnectedLines>().setName("Lines").setDescription("Draw a connected line based on a list of points");

} // namespace panda
