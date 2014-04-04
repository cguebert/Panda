#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Path.h>

namespace panda {

using types::Color;
using types::Point;
using types::Path;

class RenderLine : public Renderer
{
public:
	PANDA_CLASS(RenderLine, Renderer)

	RenderLine(PandaDocument *parent)
		: Renderer(parent)
		, inputA(initData(&inputA, "point 1", "Start of the line"))
		, inputB(initData(&inputB, "point 2", "End of the line"))
		, width(initData(&width, (PReal)1.0, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&width);
		addInput(&color);

		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const QVector<Point>& valA = inputA.getValue();
		const QVector<Point>& valB = inputB.getValue();
		const QVector<Color>& listColor = color.getValue();

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

			glLineWidth(qMax((PReal)1.0, width.getValue()));
			glBegin(GL_LINES);
			for(int i=0; i<nbLines; ++i)
			{
				glColor4fv(listColor[i % nbColor].ptr());

				if(useTwoLists)
				{
					glVertex2r(valA[i].x, valA[i].y);
					glVertex2r(valB[i].x, valB[i].y);
				}
				else
				{
					glVertex2r(valA[i*2].x, valA[i*2].y);
					glVertex2r(valA[i*2+1].x, valA[i*2+1].y);
				}
			}
			glEnd();
			glLineWidth(0);
		}
	}

protected:
	Data< QVector<Point> > inputA, inputB;
	Data<PReal> width;
	Data< QVector<Color> > color;
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
		, width(initData(&width, (PReal)1.0, "width", "Width of the line" ))
		, color(initData(&color, "color", "Color of the line"))
	{
		addInput(&input);
		addInput(&width);
		addInput(&color);

		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const QVector<Path>& paths = input.getValue();
		const QVector<Color>& listColor = color.getValue();
		int nb = paths.size();
		int nbColor = listColor.size();

		if(nb && nbColor)
		{
			if(nbColor < nb) nbColor = 1;

			glLineWidth(qMax((PReal)1.0, width.getValue()));

			for(int i=0; i<nb; ++i)
			{
				const Path& path = paths[i];
				glColor4fv(listColor[i % nbColor].ptr());

				glBegin(GL_LINE_STRIP);
				for(int j=0, nbPts=path.size(); j<nbPts; ++j)
					glVertex2d(path[j].x, path[j].y);
				glEnd();
				glLineWidth(0);
			}
		}
	}

protected:
	Data< QVector<Path> > input;
	Data<PReal> width;
	Data< QVector<Color> > color;
};

int RenderPathClass = RegisterObject<RenderPath>("Render/Path").setDescription("Draw a path");

} // namespace panda
