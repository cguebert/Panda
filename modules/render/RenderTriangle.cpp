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

class RenderTriangle : public Renderer
{
public:
	PANDA_CLASS(RenderTriangle, Renderer)

	RenderTriangle(PandaDocument *parent)
		: Renderer(parent)
		, mesh(initData(&mesh, "mesh", "Triangle to render"))
		, color(initData(&color, "color", "Color of the triangle"))
	{
		addInput(&mesh);
		addInput(&color);

		color.getAccessor().push_back(Color::black());
	}

	void render()
	{
		const Mesh& inMesh = mesh.getValue();
		const QVector<Color>& listColor = color.getValue();

		int nbTri = inMesh.getNumberOfTriangles();
		int nbColor = listColor.size();

		const QVector<Point>& pts = inMesh.getPoints();

		if(nbTri && nbColor)
		{
			if(nbColor < nbTri) nbColor = 1;

			QVector<PReal> vertices(6);
			glEnableClientState(GL_VERTEX_ARRAY);

			for(int i=0; i<nbTri; ++i)
			{
				const Mesh::Triangle& triangle = inMesh.getTriangle(i);

				glColor4fv(listColor[i % nbColor].data());

				for(int j=0; j<3; ++j)
				{
					const Point& pt = pts[triangle[j]];
					vertices[j*2  ] = pt.x;
					vertices[j*2+1] = pt.y;
				}

				glVertexPointer(2, GL_PREAL, 0, vertices.data());
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

protected:
	Data<Mesh> mesh;
	Data< QVector<Color> > color;
};

int RenderTriangleClass = RegisterObject<RenderTriangle>("Render/Filled/Triangle").setDescription("Draw a triangle");

} // namespace panda
