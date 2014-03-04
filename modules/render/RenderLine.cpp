#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <QPointF>
#include <QPainter>

#include <panda/types/Path.h>

namespace panda {

using types::Path;

class RenderLine : public Renderer
{
public:
	PANDA_CLASS(RenderLine, Renderer)

	RenderLine(PandaDocument *parent)
		: Renderer(parent)
		, inputA(initData(&inputA, "point 1", "Start of the line"))
		, inputB(initData(&inputB, "point 2", "End of the line"))
		, width(initData(&width, 1.0, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&width);
		addInput(&color);

		color.getAccessor().push_back(QColor());
	}

	void render(QPainter* painter)
	{
		const QVector<QPointF>& valA = inputA.getValue();
		const QVector<QPointF>& valB = inputB.getValue();
		double valWidth = width.getValue();
		if(!valWidth) valWidth = 1.0;
		const QVector<QColor>& listColor = color.getValue();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = qMin(valA.size(), valB.size());
		bool useTwoLists = true;
		if(nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		int nbColor = listColor.size();

		if(nbLines && nbColor)
		{
			if(nbColor < nbLines) nbColor = 1;

			painter->save();
			painter->setBrush(Qt::NoBrush);

			for(int i=0; i<nbLines; ++i)
			{
				QPen pen(listColor[i % nbColor]);
				pen.setWidthF(valWidth);
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
		const QVector<QColor>& listColor = color.getValue();

		int nbA = valA.size(), nbB = valB.size();
		int nbLines = qMin(valA.size(), valB.size());
		bool useTwoLists = true;
		if(nbA && !nbB)
		{
			useTwoLists = false;
			nbLines = nbA / 2;
		}

		int nbColor = listColor.size();

		if(nbLines && nbColor)
		{
			if(nbColor < nbLines) nbColor = 1;

			glLineWidth(qMax(1.0, width.getValue()));
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
			glLineWidth(0);
		}
	}

protected:
	Data< QVector<QPointF> > inputA, inputB;
	Data<double> width;
	Data< QVector<QColor> > color;
};

int RenderLineClass = RegisterObject<RenderLine>("Render/Line").setDescription("Draw a line between 2 points");

//*************************************************************************//

class RenderPath : public Renderer
{
public:
	PANDA_CLASS(RenderPath, Renderer)

	RenderPath(PandaDocument *parent)
		: Renderer(parent)
		, input(initData(&input, "path", "Path to be drawn"))
		, width(initData(&width, 1.0, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&input);
		addInput(&width);
		addInput(&color);

		color.getAccessor().push_back(QColor());
	}

	void render(QPainter* /*painter*/)
	{
	}

	void renderOpenGL()
	{
		const QVector<Path>& paths = input.getValue();
		const QVector<QColor>& listColor = color.getValue();
		int nb = paths.size();
		int nbColor = listColor.size();

		if(nb && nbColor)
		{
			if(nbColor < nb) nbColor = 1;

			glLineWidth(qMax(1.0, width.getValue()));

			for(int i=0; i<nb; ++i)
			{
				const Path& path = paths[i];
				QColor col = listColor[i % nbColor];
				glColor4ub(col.red(), col.green(), col.blue(), col.alpha());

				glBegin(GL_LINE_STRIP);
				for(int j=0, nbPts=path.size(); j<nbPts; ++j)
					glVertex2d(path[j].x(), path[j].y());
				glEnd();
				glLineWidth(0);
			}
		}
	}

protected:
	Data< QVector<Path> > input;
	Data<double> width;
	Data< QVector<QColor> > color;
};

int RenderPathClass = RegisterObject<RenderPath>("Render/Path").setDescription("Draw a path");

} // namespace panda
