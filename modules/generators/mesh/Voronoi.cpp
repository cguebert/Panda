#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>

#include <vector>
using std::vector;

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

namespace panda {

using types::Mesh;
using types::Point;

class GeneratorMesh_Voronoi : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Voronoi, PandaObject)

	typedef point_data<int> IPoint;
	typedef segment_data<int> Segment;
	typedef voronoi_diagram<double> Diagram;

	typedef Diagram::cell_type Cell;
	typedef Diagram::edge_type Edge;
	typedef Diagram::vertex_type Vertex;
	typedef Diagram::const_cell_iterator CellIterator;
	typedef Diagram::const_vertex_iterator VertexIterator;
	typedef Diagram::const_edge_iterator EdgeIterator;

	GeneratorMesh_Voronoi(PandaDocument *doc)
		: PandaObject(doc)
		, sites(initData(&sites, "sites", "Sites of the Voronoi tessellation"))
		, mesh(initData(&mesh, "mesh", "Mesh created from the Voronoi tessellation"))
	{
		addInput(&sites);

		addOutput(&mesh);
	}

	void update()
	{
		const QVector<Point>& pts = sites.getValue();
		auto topo = mesh.getAccessor();

		topo->clear();

		vector<IPoint> points;
		for(Point p : pts)
			points.push_back(IPoint(p.x, p.y));

		vector<Segment> segments;

		Diagram vd;
		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

		for(VertexIterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
			topo->addPoint(Point(it->x(), it->y()));

		const Vertex* firstVertex = &vd.vertices().front();
		for(EdgeIterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
		{
			if(!it->is_primary() || !it->is_linear() || !it->is_finite())
				continue;

			topo->addEdge(it->vertex0() - firstVertex, it->vertex1() - firstVertex);
		}

		cleanDirty();
	}

protected:
	Data< QVector<Point> > sites;
	Data<Mesh> mesh;
};

int GeneratorMesh_VoronoiClass = RegisterObject<GeneratorMesh_Voronoi>("Generator/Mesh/Voronoi")
		.setDescription("Create a mesh from a Voronoi tessellation");

//*************************************************************************//


} // namespace Panda
