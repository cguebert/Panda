#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/Topology.h>
#include <panda/Renderer.h>

namespace panda {

using types::Color;
using types::Point;
using types::Topology;

class RenderPolygon : public Renderer
{
public:
	PANDA_CLASS(RenderPolygon, Renderer)

	RenderPolygon(PandaDocument *parent)
		: Renderer(parent)
		, topology(initData(&topology, "topology", "Polygon to render"))
		, color(initData(&color, "color", "Color of the polygon"))
	{
		addInput(&topology);
		addInput(&color);

		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const Topology& topo = topology.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbPoly = topo.getNumberOfPolygons();
		int nbColor = listColor.size();

		const QVector<Point>& pts = topo.getPoints();

		if(nbPoly && nbColor)
		{
			if(nbColor < nbPoly) nbColor = 1;

			std::vector<PReal> vertices;
			glEnableClientState(GL_VERTEX_ARRAY);

			for(int i=0; i<nbPoly; ++i)
			{
				const Topology::Polygon& poly = topo.getPolygon(i);
				int nbPts = poly.size();
				if(!nbPts)
					continue;

				glColor4fv(listColor[i % nbColor].ptr());

				vertices.resize(nbPts * 2);

				for(int j=0; j<nbPts; ++j)
				{
					const Point& pt = pts[poly[j]];
					vertices[j*2  ] = pt.x;
					vertices[j*2+1] = pt.y;
				}

				glVertexPointer(2, GL_PREAL, 0, vertices.data());
				glDrawArrays(GL_TRIANGLE_FAN, 0, nbPts);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data<Topology> topology;
	Data< QVector<Color> > color;
};

int RenderPolygonClass = RegisterObject<RenderPolygon>("Render/Polygon").setDescription("Draw a polygon");

} // namespace panda
