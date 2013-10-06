#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Topology.h>

#include <vector>
using std::vector;

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

namespace panda {

using types::Topology;

class GeneratorTopology_Voronoi : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_Voronoi, PandaObject)

	typedef point_data<int> Point;
	typedef segment_data<int> Segment;
	typedef voronoi_diagram<double> Diagram;

	typedef Diagram::cell_type Cell;
	typedef Diagram::edge_type Edge;
	typedef Diagram::vertex_type Vertex;
	typedef Diagram::const_cell_iterator CellIterator;
	typedef Diagram::const_vertex_iterator VertexIterator;
	typedef Diagram::const_edge_iterator EdgeIterator;

	GeneratorTopology_Voronoi(PandaDocument *doc)
		: PandaObject(doc)
		, sites(initData(&sites, "sites", "Sites of the Voronoi tessellation"))
		, topology(initData(&topology, "topology", "Topology created from the Voronoi tessellation"))
	{
		addInput(&sites);

		addOutput(&topology);
	}

	void update()
	{
		const QVector<QPointF>& pts = sites.getValue();
		auto topo = topology.getAccessor();

		topo->clear();

		vector<Point> points;
		foreach(QPointF p, pts)
			points.push_back(Point(p.x(), p.y()));

		vector<Segment> segments;

		Diagram vd;
		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

		for(VertexIterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
			topo->addPoint(QPointF(it->x(), it->y()));

		const Vertex* firstVertex = &vd.vertices().front();
		for(EdgeIterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
		{
			if(!it->is_primary() || !it->is_linear() || !it->is_finite())
				continue;

			topo->addEdge(it->vertex0() - firstVertex, it->vertex1() - firstVertex);
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > sites;
	Data<Topology> topology;
};

int GeneratorTopology_VoronoiClass = RegisterObject<GeneratorTopology_Voronoi>("Generator/Topology/Voronoi")
		.setDescription("Create a topology from a Voronoi tessellation");

//*************************************************************************//


} // namespace Panda
