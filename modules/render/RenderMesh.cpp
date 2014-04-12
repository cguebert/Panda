#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Color.h>
#include <panda/types/Mesh.h>
#include <panda/Renderer.h>

namespace panda {

using types::Color;
using types::Point;
using types::Mesh;

class RenderMesh : public Renderer
{
public:
	PANDA_CLASS(RenderMesh, Renderer)

	RenderMesh(PandaDocument *parent)
		: Renderer(parent)
		, mesh(initData(&mesh, "mesh", "Mesh to render"))
		, color(initData(&color, "color", "Color of the points"))
	{
		addInput(&mesh);
		addInput(&color);

		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const Mesh& topo = mesh.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbPts = topo.getNumberOfPoints();
		int nbPoly = topo.getNumberOfPolygons();
		int nbColor = listColor.size();

		const QVector<Point>& pts = topo.getPoints();

		if(nbPoly && nbColor)
		{
			if(nbColor < nbPts) nbColor = 1;

			std::vector<PReal> vertices;
			std::vector<float> colors;
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			for(int i=0; i<nbPoly; ++i)
			{
				const Mesh::Polygon& poly = topo.getPolygon(i);
				int nbPts = poly.size();
				if(!nbPts)
					continue;

				vertices.resize(nbPts * 2);
				colors.resize(nbPts * 4);

				for(int j=0; j<nbPts; ++j)
				{
					const Point& pt = pts[poly[j]];
					vertices[j*2  ] = pt.x;
					vertices[j*2+1] = pt.y;

					Color valCol = listColor[poly[j] % nbColor];
					for(int k=0; k<4; ++k)
						colors[j*4+k] = valCol[k];
				}

				glVertexPointer(2, GL_PREAL, 0, vertices.data());
				glColorPointer(4, GL_FLOAT, 0, colors.data());
				glDrawArrays(GL_TRIANGLE_FAN, 0, nbPts);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Color> > color;
};

int RenderMeshClass = RegisterObject<RenderMesh>("Render/Mesh").setDescription("Draw a mesh");

} // namespace panda
