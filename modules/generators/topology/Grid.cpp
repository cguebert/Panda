#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Rect.h>
#include <panda/types/Topology.h>

namespace panda {

using types::Point;
using types::Rect;
using types::Topology;

class GeneratorTopology_Grid : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_Grid, PandaObject)


	GeneratorTopology_Grid(PandaDocument *doc)
		: PandaObject(doc)
		, area(initData(&area, Rect(100, 100, 200, 200), "area", "Position and size of the grid"))
		, nbX(initData(&nbX, 5, "nbX", "Number of cells horizontally"))
		, nbY(initData(&nbY, 5, "nbY", "Number of cells vertically"))
		, triangulate(initData(&triangulate, 0, "triangulate", "Create triangles instead of quads"))
		, topology(initData(&topology, "topology", "Topology created"))
	{
		addInput(&area);
		addInput(&nbX);
		addInput(&nbY);
		addInput(&triangulate);
		triangulate.setWidget("checkbox");

		addOutput(&topology);
	}

	void update()
	{
		Rect bounds = area.getValue();
		int nx = nbX.getValue(), ny = nbY.getValue();
		bool tri = triangulate.getValue();
		auto topo = topology.getAccessor();
		topo->clear();

		qreal dx = (nx > 0 ? dx = bounds.width() / (nx-1) : 0);
		qreal dy = (ny > 0 ? dy = bounds.height() / (ny-1) : 0);

		for(int y=0; y<ny; ++y)
			for(int x=0; x<nx; ++x)
				topo->addPoint(Point(bounds.left() + x * dx, bounds.top() + y * dy));

		for(int y=1; y<ny; ++y)
		{
			for(int x=1; x<nx; ++x)
			{
				if(tri)
				{
					Topology::Polygon poly;
					poly.push_back((y-1)*nx + x);
					poly.push_back((y-1)*nx + x - 1);
					poly.push_back(y*nx + x - 1);
					topo->addPolygon(poly);

					poly.clear();
					poly.push_back((y-1)*nx + x);
					poly.push_back(y*nx + x - 1);
					poly.push_back(y*nx + x);
					topo->addPolygon(poly);
				}
				else
				{
					Topology::Polygon poly;
					poly.push_back((y-1)*nx + x);
					poly.push_back((y-1)*nx + x - 1);
					poly.push_back(y*nx + x - 1);
					poly.push_back(y*nx + x);
					topo->addPolygon(poly);
				}
			}
		}


		topo->createEdgeList();

		cleanDirty();
	}

protected:
	Data<Rect> area;
	Data<int> nbX, nbY, triangulate;
	Data<Topology> topology;
};

int GeneratorTopology_GridClass = RegisterObject<GeneratorTopology_Grid>("Generator/Topology/Grid")
		.setDescription("Create a topology from a regular grid");

//*************************************************************************//


} // namespace Panda
