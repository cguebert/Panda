#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>

#include <cmath>

namespace panda {

using types::Color;
using types::Point;

class RenderCircle : public Renderer
{
public:
	PANDA_CLASS(RenderCircle, Renderer)

	RenderCircle(PandaDocument *parent)
		: Renderer(parent)
		, center(initData(&center, "center", "Center position of the circle"))
		, radius(initData(&radius, "radius", "Radius of the circle" ))
		, lineWidth(initData(&lineWidth, "lineWidth", "Width of the line"))
		, color(initData(&color, "color", "Color of the circle"))
	{
		addInput(&center);
		addInput(&radius);
		addInput(&lineWidth);
		addInput(&color);

		center.getAccessor().push_back(Point(100, 100));
		radius.getAccessor().push_back(5.0);
		lineWidth.getAccessor().push_back(0.0);
		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const QVector<Point>& listCenter = center.getValue();
		const QVector<PReal>& listRadius = radius.getValue();
		const QVector<PReal>& listWidth = lineWidth.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbCenter = listCenter.size();
		int nbRadius = listRadius.size();
		int nbWidth = listWidth.size();
		int nbColor = listColor.size();

		if(nbCenter && nbRadius && nbColor && nbWidth)
		{
			if(nbRadius < nbCenter) nbRadius = 1;
			if(nbWidth < nbCenter) nbWidth = 1;
			if(nbColor < nbCenter) nbColor = 1;
			std::vector<PReal> vertices;

			glEnableClientState(GL_VERTEX_ARRAY);
			for(int i=0; i<nbCenter; ++i)
			{
				glColor4fv(listColor[i % nbColor].data());
				glLineWidth(listWidth[i % nbWidth]);

				PReal valRadius = listRadius[i % nbRadius];
				int nbSeg = static_cast<int>(floor(valRadius * M_PI * 2));
				if(nbSeg < 3) continue;
				vertices.resize((nbSeg + 2) * 2);

				PReal angle = M_PI * 2 / nbSeg;
				PReal ca = cos(angle), sa = sin(angle);
				Point dir = Point(valRadius, 0);
				Point valCenter = listCenter[i];
				for(int i=0; i<nbSeg; i++)
				{
					Point pt = Point(dir.x*ca+dir.y*sa, dir.y*ca-dir.x*sa);
					vertices[i*2  ] = valCenter.x + pt.x;
					vertices[i*2+1] = valCenter.y + pt.y;
					dir = pt;
				}
				glVertexPointer(2, GL_PREAL, 0, vertices.data());
				glDrawArrays(GL_LINE_LOOP, 0, nbSeg);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data< QVector<Point> > center;
	Data< QVector<PReal> > radius, lineWidth;
	Data< QVector<Color> > color;
};

int RenderCircleClass = RegisterObject<RenderCircle>("Render/Line/Circle").setDescription("Draw a circle");

} // namespace panda
