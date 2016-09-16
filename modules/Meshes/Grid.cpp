#include <panda/object/ObjectFactory.h>

#include <panda/types/Rect.h>
#include <panda/types/Mesh.h>

namespace panda {

using types::Point;
using types::Rect;
using types::Mesh;

class GeneratorMesh_Grid : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Grid, PandaObject)


	GeneratorMesh_Grid(PandaDocument *doc)
		: PandaObject(doc)
		, area(initData(Rect(100, 100, 200, 200), "area", "Position and size of the grid"))
		, nbX(initData(5, "nbX", "Number of cells horizontally"))
		, nbY(initData(5, "nbY", "Number of cells vertically"))
		, mesh(initData("mesh", "Mesh created"))
	{
		addInput(area);
		addInput(nbX);
		addInput(nbY);

		addOutput(mesh);
	}

	void update()
	{
		Rect bounds = area.getValue();
		int nx = nbX.getValue(), ny = nbY.getValue();
		auto outMesh = mesh.getAccessor();
		outMesh->clear();

		float dx = (nx > 0 ? dx = bounds.width() / (nx-1) : 0);
		float dy = (ny > 0 ? dy = bounds.height() / (ny-1) : 0);

		for(int y=0; y<ny; ++y)
			for(int x=0; x<nx; ++x)
				outMesh->addPoint(Point(bounds.left() + x * dx, bounds.top() + y * dy));

		for(int y=1; y<ny; ++y)
		{
			for(int x=1; x<nx; ++x)
			{
				outMesh->addTriangle((y-1)*nx + x,
								  (y-1)*nx + x - 1,
								  y*nx + x - 1);

				outMesh->addTriangle((y-1)*nx + x,
								  y*nx + x - 1,
								  y*nx + x);
			}
		}


		outMesh->createEdgeList();
	}

protected:
	Data<Rect> area;
	Data<int> nbX, nbY;
	Data<Mesh> mesh;
};

int GeneratorMesh_GridClass = RegisterObject<GeneratorMesh_Grid>("Generator/Mesh/Grid")
		.setDescription("Create a mesh from a regular grid");

//****************************************************************************//


} // namespace Panda
