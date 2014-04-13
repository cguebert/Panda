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
		const Mesh& inMesh = mesh.getValue();
		QVector<Color> listColor = color.getValue();

		int nbPts = inMesh.getNumberOfPoints();
		int nbTri = inMesh.getNumberOfTriangles();
		int nbColor = listColor.size();

		if(nbTri && nbColor)
		{
			if(nbColor < nbPts)
				listColor.fill(listColor[0], nbPts);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			glVertexPointer(2, GL_PREAL, 0, inMesh.getPoints().data());
			glColorPointer(4, GL_FLOAT, 0, listColor.data());
			glDrawElements(GL_TRIANGLES, nbTri * 3, GL_UNSIGNED_INT, inMesh.getTriangles().data());

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
