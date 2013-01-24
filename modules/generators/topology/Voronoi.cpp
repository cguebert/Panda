#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/Topology.h>

#include <vector>
using std::vector;

#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::point_data;
using boost::polygon::segment_data;

namespace panda {

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
		Topology& topo = *topology.beginEdit();

		topo.clear();

		vector<Point> points;
		foreach(QPointF p, pts)
			points.push_back(Point(p.x(), p.y()));

		vector<Segment> segments;

		Diagram vd;
		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

		for(VertexIterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it)
			topo.addPoint(QPointF(it->x(), it->y()));

		const Vertex* firstVertex = &vd.vertices().front();
		for(EdgeIterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
		{
			if(!it->is_primary() || !it->is_linear() || !it->is_finite())
				continue;

			topo.addEdge(it->vertex0() - firstVertex, it->vertex1() - firstVertex);
		}
/*
		int result = 0;
		for(Diagram::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it)
		{
			const Diagram::cell_type &cell = *it;
			const Diagram::edge_type *edge = cell.incident_edge();
			do {
				if(edge->is_linear())
					++result;
				edge = edge->next();
			} while (edge != cell.incident_edge());
		 }*/

		topology.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<QPointF> > sites;
	Data<Topology> topology;
};

int GeneratorTopology_VoronoiClass = RegisterObject("Generator/Topology/Voronoi").setClass<GeneratorTopology_Voronoi>().setDescription("Create a topology from a Voronoi tessellation");

//*************************************************************************//


} // namespace Panda
