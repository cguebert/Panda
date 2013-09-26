#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/Topology.h>

#include <vector>
using std::vector;

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

#include <QMap>

namespace panda {

class GeneratorTopology_Delaunay : public PandaObject
{
public:
	PANDA_CLASS(GeneratorTopology_Delaunay, PandaObject)

	typedef point_data<int> Point;
	typedef segment_data<int> Segment;
	typedef voronoi_diagram<double> Diagram;

	typedef Diagram::cell_type Cell;
	typedef Diagram::edge_type Edge;
	typedef Diagram::vertex_type Vertex;
	typedef Diagram::const_cell_iterator CellIterator;
	typedef Diagram::const_vertex_iterator VertexIterator;
	typedef Diagram::const_edge_iterator EdgeIterator;

	GeneratorTopology_Delaunay(PandaDocument *doc)
		: PandaObject(doc)
		, document(doc)
		, vertices(initData(&vertices, "vertices", "Sites of the Delaunay triangulation"))
		, topology(initData(&topology, "topology", "Topology created from the Delaunay triangulation"))
	{
		addInput(&vertices);

		addOutput(&topology);
	}

	void update()
	{
		const QVector<QPointF>& pts = vertices.getValue();
		auto topo = topology.getAccessor();

		topo->clear();

		vector<Point> points;
		foreach(QPointF p, pts)
			points.push_back(Point(p.x(), p.y()));

		vector<Segment> segments;

		Diagram vd;
		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

		topo->addPoints(pts);

		QRectF area = QRectF(QPointF(0,0), document->getRenderSize());

		for(EdgeIterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
		{
			const Cell* c1 = it->cell();
			bool bp1 = false, bp2 = false;
			if(it->vertex0())
			{
				QPointF pt = QPointF(it->vertex0()->x(), it->vertex0()->y());
				bp1 = area.contains(pt);
			}
			if(it->vertex1())
			{
				QPointF pt = QPointF(it->vertex1()->x(), it->vertex1()->y());
				bp2 = area.contains(pt);
			}
			if(it->twin() && (bp1 || bp2))
			{
				const Cell* c2 = it->twin()->cell();
				Topology::Edge e(c1->source_index(), c2->source_index());
				if(topo->getEdgeIndex(e) == -1)
					topo->addEdge(e);
			}
		}

		topo->createTriangles();

		this->cleanDirty();
	}

protected:
	PandaDocument *document;
	Data< QVector<QPointF> > vertices;
	Data<Topology> topology;
};

int GeneratorTopology_DelaunayClass = RegisterObject("Generator/Topology/Delaunay")
		.setClass<GeneratorTopology_Delaunay>()
		.setDescription("Create a topology from a Delaunay triangulation");

//*************************************************************************//


} // namespace Panda
