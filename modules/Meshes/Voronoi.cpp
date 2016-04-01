#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>

#pragma warning ( disable: 4267 )

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

namespace panda {

using types::Path;
using types::Point;
using types::Rect;

class VoronoiHelper
{
public:
	static std::vector<Path> computeVoronoi(const std::vector<Point>& points, const Rect& boundingBox)
	{
		VoronoiHelper helper(points, boundingBox);
		helper.doVoronoi();
		return helper.m_paths;
	}

private:
	using IPoint = point_data<int>;
	using Segment = segment_data<int>;
	using Diagram = voronoi_diagram<double>;

	using Cell = Diagram::cell_type;
	using Edge = Diagram::edge_type;
	using Vertex = Diagram::vertex_type;
	using CellIterator = Diagram::const_cell_iterator;
	using VertexIterator = Diagram::const_vertex_iterator;
	using EdgeIterator = Diagram::const_edge_iterator;

	VoronoiHelper(const std::vector<Point>& points, const Rect& boundingBox)
		: m_points(points), m_boundingBox(boundingBox)
	{
		m_paths.resize(m_points.size());

		m_maxSide = std::max(m_boundingBox.width(), m_boundingBox.height());
	}

	void createDiagram(Diagram& vd)
	{
		std::vector<IPoint> points;
		for (Point p : m_points)
			points.push_back(IPoint(static_cast<int>(p.x), static_cast<int>(p.y)));

		std::vector<Segment> segments;

		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);
	}

	template <class PT>
	inline Point convert(PT* v)
	{ return Point(static_cast<float>(v->x()), static_cast<float>(v->y())); }

	void clipInfiniteEdge(const Edge& edge, std::vector<Point>& points)
	{
		const Cell& cell1 = *edge.cell();
		const Cell& cell2 = *edge.twin()->cell();

		Point p1 = m_points[cell1.source_index()];
		Point p2 = m_points[cell2.source_index()];
		Point origin = (p1 + p2) / 2;
		Point dir(p1.y - p2.y, p2.x - p1.x);
	
		float coef = m_maxSide / std::max(fabs(dir.x), fabs(dir.y));
		if (edge.vertex0()) 
			points.push_back(convert(edge.vertex0()));
		else
			points.push_back(origin - dir * coef);

		if (edge.vertex1())
			points.push_back(convert(edge.vertex1()));
		else
			points.push_back(origin + dir * coef);
	}

	void doVoronoi()
	{
		Diagram vd;
		createDiagram(vd);

		for (const auto& cell : vd.cells())
		{
			const auto firstEdge = cell.incident_edge();
			auto edge = firstEdge;
			Path path;

			do {
				if (edge->is_primary() && edge->is_linear())
				{
					if (edge->is_finite())
					{
						path.points.push_back(convert(edge->vertex0()));
						path.points.push_back(convert(edge->vertex1()));
					}
					else
						clipInfiniteEdge(*edge, path.points);
				}

				edge = edge->next();
			} while (edge != firstEdge);

			if (!path.points.empty())
				m_paths[cell.source_index()] = std::move(path);
		}
	}

	std::vector<Point> m_points;
	std::vector<Path> m_paths;
	Rect m_boundingBox;
	float m_maxSide = 0;
};

class GeneratorMesh_Voronoi : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Voronoi, PandaObject)

	GeneratorMesh_Voronoi(PandaDocument *doc)
		: PandaObject(doc)
		, m_sites(initData("sites", "Sites of the Voronoi tessellation"))
		, m_boundingBox(initData("bounding box", "The polygon will be clipped to this rectangle. If null, will use the render area."))
		, m_paths(initData("polygons", "Polygons created from the Voronoi tessellation"))
	{
		addInput(m_sites);
		addInput(m_boundingBox);
		addOutput(m_paths);
	}

	void update()
	{
		
		auto boundingBox = m_boundingBox.getValue();
		if (boundingBox.empty())
		{
			auto size = parentDocument()->getRenderSize();
			boundingBox.set(0, 0, static_cast<float>(size.width()), static_cast<float>(size.height()));
		}

		const std::vector<Point>& pts = m_sites.getValue();
		auto acc = m_paths.getAccessor();
		acc.wref() = VoronoiHelper::computeVoronoi(pts, boundingBox);
	}

protected:
	Data< std::vector<Point> > m_sites;
	Data< Rect > m_boundingBox;
	Data< std::vector<Path> > m_paths;
};

int GeneratorMesh_VoronoiClass = RegisterObject<GeneratorMesh_Voronoi>("Generator/Mesh/Voronoi")
		.setDescription("Create a mesh from a Voronoi tessellation");

//****************************************************************************//


} // namespace Panda
