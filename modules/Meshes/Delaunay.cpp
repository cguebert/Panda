#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <panda/types/Mesh.h>
#include <panda/types/Rect.h>

#include <vector>
using std::vector;

#pragma warning ( disable: 4267 )

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

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
		, vertices(initData("vertices", "Sites of the Delaunay triangulation"))
		, mesh(initData("mesh", "Mesh created from the Delaunay triangulation"))
	{
		addInput(vertices);

		addOutput(mesh);
	}

	void update()
	{
		const std::vector<Point>& pts = vertices.getValue();
		auto outMesh = mesh.getAccessor();

		outMesh->clear();

		vector<IPoint> points;
		for(const auto& p : pts)
			points.push_back(IPoint(static_cast<int>(p.x), static_cast<int>(p.y)));

		vector<Segment> segments;

		Diagram vd;
		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

		outMesh->addPoints(pts);

		auto s = m_parentDocument->getRenderSize();
		Rect area = Rect(0, 0, static_cast<float>(s.width()), static_cast<float>(s.height()));

		for(EdgeIterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
		{
			const Cell* c1 = it->cell();
			bool bp1 = false, bp2 = false;
			if(it->vertex0())
			{
				Point pt = Point(static_cast<float>(it->vertex0()->x()), static_cast<float>(it->vertex0()->y()));
				bp1 = area.contains(pt);
			}
			if(it->vertex1())
			{
				Point pt = Point(static_cast<float>(it->vertex1()->x()), static_cast<float>(it->vertex1()->y()));
				bp2 = area.contains(pt);
			}
			if(it->twin() && (bp1 || bp2))
			{
				const Cell* c2 = it->twin()->cell();
				Mesh::Edge e = Mesh::makeEdge(c1->source_index(), c2->source_index());
				if(outMesh->getEdgeIndex(e) == Mesh::InvalidID)
					outMesh->addEdge(e);
			}
		}

		outMesh->createTriangles();

		cleanDirty();
	}

protected:
	Data< std::vector<Point> > vertices;
	Data<Mesh> mesh;
};

int GeneratorMesh_DelaunayClass = RegisterObject<GeneratorMesh_Delaunay>("Generator/Mesh/Delaunay")
		.setDescription("Create a mesh from a Delaunay triangulation");

//****************************************************************************//


} // namespace Panda
