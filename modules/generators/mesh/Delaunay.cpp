#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Mesh.h>
#include <panda/types/Rect.h>

#include <vector>
using std::vector;

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

#include <QMap>

namespace panda {

using types::Mesh;
using types::Point;
using types::Rect;

class GeneratorMesh_Delaunay : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Delaunay, PandaObject)

	typedef point_data<int> IPoint;
	typedef segment_data<int> Segment;
	typedef voronoi_diagram<double> Diagram;

	typedef Diagram::cell_type Cell;
	typedef Diagram::edge_type Edge;
	typedef Diagram::vertex_type Vertex;
	typedef Diagram::const_cell_iterator CellIterator;
	typedef Diagram::const_vertex_iterator VertexIterator;
	typedef Diagram::const_edge_iterator EdgeIterator;

	GeneratorMesh_Delaunay(PandaDocument *doc)
		: PandaObject(doc)
		, vertices(initData(&vertices, "vertices", "Sites of the Delaunay triangulation"))
		, mesh(initData(&mesh, "mesh", "Mesh created from the Delaunay triangulation"))
	{
		addInput(&vertices);

		addOutput(&mesh);
	}

	void update()
	{
		const QVector<Point>& pts = vertices.getValue();
		auto topo = mesh.getAccessor();

		topo->clear();

		vector<IPoint> points;
		for(const auto& p : pts)
			points.push_back(IPoint(p.x, p.y));

		vector<Segment> segments;

		Diagram vd;
		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

		topo->addPoints(pts);

		QSize s = parentDocument->getRenderSize();
		Rect area = Rect(0, 0, s.width(), s.height());

		for(EdgeIterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
		{
			const Cell* c1 = it->cell();
			bool bp1 = false, bp2 = false;
			if(it->vertex0())
			{
				Point pt = Point(it->vertex0()->x(), it->vertex0()->y());
				bp1 = area.contains(pt);
			}
			if(it->vertex1())
			{
				Point pt = Point(it->vertex1()->x(), it->vertex1()->y());
				bp2 = area.contains(pt);
			}
			if(it->twin() && (bp1 || bp2))
			{
				const Cell* c2 = it->twin()->cell();
				Mesh::Edge e = Mesh::makeEdge(c1->source_index(), c2->source_index());
				if(topo->getEdgeIndex(e) == Mesh::InvalidID)
					topo->addEdge(e);
			}
		}

		topo->createTriangles();

		cleanDirty();
	}

protected:
	Data< QVector<Point> > vertices;
	Data<Mesh> mesh;
};

int GeneratorMesh_DelaunayClass = RegisterObject<GeneratorMesh_Delaunay>("Generator/Mesh/Delaunay")
		.setDescription("Create a mesh from a Delaunay triangulation");

//*************************************************************************//


} // namespace Panda
