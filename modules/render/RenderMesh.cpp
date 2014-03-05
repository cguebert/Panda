#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Topology.h>
#include <panda/Renderer.h>
#include <QPainter>

namespace panda {

using types::Topology;

class RenderMesh : public Renderer
{
public:
	PANDA_CLASS(RenderMesh, Renderer)

	RenderMesh(PandaDocument *parent)
		: Renderer(parent)
		, topology(initData(&topology, "topology", "Topology to render"))
		, color(initData(&color, "color", "Color of the points"))
	{
		addInput(&topology);
		addInput(&color);

		color.getAccessor().push_back(QColor());
	}

	void render(QPainter* /*painter*/)
	{
	}

	void renderOpenGL()
	{
		const Topology& topo = topology.getValue();
		const QVector<QColor>& listColor = color.getValue();

		int nbPts = topo.getNumberOfPoints();
		int nbPoly = topo.getNumberOfPolygons();
		int nbColor = listColor.size();

		const QVector<QPointF>& pts = topo.getPoints();

		if(nbPoly && nbColor)
		{
			if(nbColor < nbPts) nbColor = 1;

			std::vector<double> vertices;
			std::vector<unsigned char> colors;
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			for(int i=0; i<nbPoly; ++i)
			{
				const Topology::Polygon& poly = topo.getPolygon(i);
				int nbPts = poly.size();
				if(!nbPts)
					continue;

				vertices.resize(nbPts * 2);
				colors.resize(nbPts * 4);

				for(int j=0; j<nbPts; ++j)
				{
					const QPointF& pt = pts[poly[j]];
					vertices[j*2  ] = pt.x();
					vertices[j*2+1] = pt.y();

					QColor valCol = listColor[poly[j] % nbColor];
					colors[j*4  ] = valCol.red();
					colors[j*4+1] = valCol.green();
					colors[j*4+2] = valCol.blue();
					colors[j*4+3] = valCol.alpha();
				}

				glVertexPointer(2, GL_DOUBLE, 0, vertices.data());
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors.data());
				glDrawArrays(GL_TRIANGLE_FAN, 0, nbPts);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}
	}

protected:
	Data<Topology> topology;
	Data< QVector<QColor> > color;
};

int RenderMeshClass = RegisterObject<RenderMesh>("Render/Mesh").setDescription("Draw a mesh");

} // namespace panda
