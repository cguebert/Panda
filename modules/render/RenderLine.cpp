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
		, inputB(initData(&inputB, "point 2", "End of the line"))
		, width(initData(&width, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&width);
		addInput(&color);

		width.getAccessor().push_back(0.0);
		color.getAccessor().push_back(QColor());
	}

	void render(QPainter* painter)
	{
		const QVector<QPointF>& valA = inputA.getValue();
		const QVector<QPointF>& valB = inputB.getValue();
		const QVector<double>& listWidth = width.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = qMin(valA.size(), valB.size());
		bool useTwoLists = true;
		if(nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		if(nbLines && nbWidth && nbColor)
		{
			if(nbWidth < nbLines) nbWidth = 1;
			if(nbColor < nbLines) nbColor = 1;

			painter->save();
			painter->setBrush(Qt::NoBrush);

			for(int i=0; i<nbLines; ++i)
			{
				QPen pen(listColor[i % nbColor]);
				pen.setWidthF(listWidth[i % nbWidth]);
				pen.setCapStyle(Qt::RoundCap);
				painter->setPen(pen);

				if(useTwoLists)
					painter->drawLine(valA[i], valB[i]);
				else
					painter->drawLine(valA[i*2], valA[i*2+1]);
			}

			painter->restore();
		}
	}

	void renderOpenGL()
	{
		const QVector<QPointF>& valA = inputA.getValue();
		const QVector<QPointF>& valB = inputB.getValue();
		const QVector<double>& listWidth = width.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = qMin(valA.size(), valB.size());
		bool useTwoLists = true;
		if(nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		if(nbLines && nbWidth && nbColor)
		{
			if(nbWidth < nbLines) nbWidth = 1;
			if(nbColor < nbLines) nbColor = 1;

			glBegin(GL_LINES);
			for(int i=0; i<nbLines; ++i)
			{
				QColor col = listColor[i % nbColor];
				glColor4ub(col.red(), col.green(), col.blue(), col.alpha());

				if(useTwoLists)
				{
					glVertex2d(valA[i].x(), valA[i].y());
					glVertex2d(valB[i].x(), valB[i].y());
				}
				else
				{
					glVertex2d(valA[i*2].x(), valA[i*2].y());
					glVertex2d(valA[i*2+1].x(), valA[i*2+1].y());
				}
			}
			glEnd();
		}
	}

protected:
	Data< QVector<QPointF> > inputA, inputB;
	Data< QVector<double> > width;
	Data< QVector<QColor> > color;
};

int RenderLineClass = RegisterObject<RenderLine>("Render/Line").setDescription("Draw a line between 2 points");

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

	void renderOpenGL()
	{
		const QVector<QPointF>& points = input.getValue();
		int nb = points.size();

		if(nb)
		{
			glLineWidth(width.getValue());
			QColor col = color.getValue();
			glColor4ub(col.red(), col.green(), col.blue(), col.alpha());

			glBegin(GL_LINE_STRIP);
			for(int i=0; i<nb; ++i)
				glVertex2d(points[i].x(), points[i].y());
			glEnd();

			glLineWidth(0);
		}
	}

protected:
	Data< QVector<QPointF> > input;
	Data<double> width;
	Data<QColor> color;
};

int RenderConnectedLinesClass = RegisterObject<RenderConnectedLines>("Render/Connected lines").setDescription("Draw a connected line based on a list of points");

} // namespace panda
