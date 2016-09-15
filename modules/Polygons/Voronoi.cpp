#include <panda/document/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
namespace gtl = boost::polygon;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244) /* conversion from 'type1' to 'type2', possible loss of data */
#endif

#define JC_VORONOI_IMPLEMENTATION
#include <modules/Polygons/libs/jc_voronoi.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
namespace bg = boost::geometry;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
	const float toGTLFactor = 100;
	const float toPandaFactor = 1.f / toGTLFactor;
	using IntPoint = gtl::point_data<int>;
	using IntPointsList = std::vector<IntPoint>;
	using Segment = gtl::segment_data<int>;
	using Diagram = gtl::voronoi_diagram<double>;

	using Cell = Diagram::cell_type;
	using Edge = Diagram::edge_type;
	using Vertex = Diagram::vertex_type;
	using CellIterator = Diagram::const_cell_iterator;
	using VertexIterator = Diagram::const_vertex_iterator;
	using EdgeIterator = Diagram::const_edge_iterator;

	using BGPoint = bg::model::d2::point_xy<float>;
	using BGPointsList = std::vector<BGPoint>;
	using BGPoly = bg::model::polygon<BGPoint, false>;

	VoronoiHelper(const std::vector<Point>& points, const Rect& boundingBox)
		: m_points(points), m_boundingBox(boundingBox)
	{
		m_paths.resize(m_points.size());

		BGPointsList pts;
		pts.emplace_back(m_boundingBox.left(), m_boundingBox.top());
		pts.emplace_back(m_boundingBox.right(), m_boundingBox.top());
		pts.emplace_back(m_boundingBox.right(), m_boundingBox.bottom());
		pts.emplace_back(m_boundingBox.left(), m_boundingBox.bottom());
		pts.emplace_back(m_boundingBox.left(), m_boundingBox.top());
		bg::append(m_bbPoly, pts);

		m_maxSide = std::max(m_boundingBox.width(), m_boundingBox.height());
	}

	inline IntPointsList convert(const std::vector<Point>& input)
	{
		IntPointsList points;
		for (const auto p : input)
			points.emplace_back(static_cast<int>(p.x * toGTLFactor), static_cast<int>(p.y * toGTLFactor));
		return points;
	}

	template <class PT>
	inline Point convert(PT* v)
	{ return Point(static_cast<float>(v->x() * toPandaFactor), static_cast<float>(v->y() * toPandaFactor)); }

	void createDiagram(Diagram& vd)
	{
		std::vector<Segment> segments;
		IntPointsList points = convert(m_points);

		construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);
	}

	inline void addPoint(std::vector<Point>& points, Point pt)
	{
		if (points.empty() || points.back() != pt)
		{
			points.push_back(pt);

			if (pt.x < m_boundingBox.left() || pt.x > m_boundingBox.right()
				|| pt.y < m_boundingBox.top() || pt.y > m_boundingBox.bottom())
				m_mustClipPolygon = true;
		}
	}

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
			addPoint(points, convert(edge.vertex0()));
		else
			addPoint(points, origin - dir * coef);

		if (edge.vertex1())
			addPoint(points, convert(edge.vertex1()));
		else
			addPoint(points, origin + dir * coef);
	}

	void clipPolygon(std::vector<Point>& points)
	{
		BGPointsList pts;
		for (const Point& pt : points)
			pts.emplace_back(pt.x, pt.y);
		BGPoly poly;
		bg::append(poly, pts);

		std::vector<BGPoly> result;
		boost::geometry::intersection(m_bbPoly, poly, result);

		points.clear();
		if (!result.empty())
		{
			for (const auto& pt : result.front().outer())
				points.emplace_back(pt.x(), pt.y());
		}
	}

	void doVoronoi()
	{
		Diagram vd;
		createDiagram(vd);

		for (const auto& cell : vd.cells())
		{
			m_mustClipPolygon = false;
			const auto firstEdge = cell.incident_edge();
			auto edge = firstEdge;
			Path path;

			do {
				if (edge->is_primary() && edge->is_linear())
				{
					if (edge->is_finite())
					{
						addPoint(path.points, convert(edge->vertex0()));
						addPoint(path.points, convert(edge->vertex1()));
					}
					else
						clipInfiniteEdge(*edge, path.points);
				}

				edge = edge->next();
			} while (edge != firstEdge);

			if (!path.points.empty())
			{
				if (m_mustClipPolygon)
					clipPolygon(path.points);
				m_paths[cell.source_index()] = std::move(path);
			}
		}
	}

	std::vector<Point> m_points;
	std::vector<Path> m_paths;

	Rect m_boundingBox;
	BGPoly m_bbPoly;
	float m_maxSide = 0;
	bool m_mustClipPolygon = false;
};

//****************************************************************************//

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

class GeneratorMesh_Voronoi2 : public PandaObject
{
public:
	PANDA_CLASS(GeneratorMesh_Voronoi2, PandaObject)

	GeneratorMesh_Voronoi2(PandaDocument *doc)
		: PandaObject(doc)
		, m_sites(initData("sites", "Sites of the Voronoi tessellation"))
		, m_paths(initData("polygons", "Polygons created from the Voronoi tessellation"))
	{
		addInput(m_sites);
		addOutput(m_paths);
	}

	static inline Point convert(const jcv_point& pt)
	{ return{ pt.x, pt.y }; }

	void update()
	{
		auto size = parentDocument()->getRenderSize();

		const std::vector<Point>& pts = m_sites.getValue();
		auto acc = m_paths.getAccessor();
		acc.clear();

		if (pts.empty())
			return;
		
		jcv_diagram diagram;
		memset(&diagram, 0, sizeof(jcv_diagram));
		jcv_diagram_generate(pts.size(), reinterpret_cast<const jcv_point*>(pts.data()),
							 static_cast<int>(size.width()), static_cast<int>(size.height()),
							 &diagram);

		const auto sites = jcv_diagram_get_sites(&diagram);
		for (int i = 0; i < diagram.numsites; ++i)
		{
			const jcv_site& site = sites[i];
			const jcv_graphedge* e = site.edges;

			Path path;
			while (e)
			{
				if (path.points.empty() || (path.points.back() - convert(e->pos[0])).norm2() > 1e-4)
					path.points.push_back(convert(e->pos[0]));
				path.points.push_back(convert(e->pos[1]));
				e = e->next;
			}
			path.points.back() = path.points.front(); // We want to be sure the values are exactly the same
			acc.push_back(path);
		}

		jcv_diagram_free(&diagram);
	}

protected:
	Data< std::vector<Point> > m_sites;
	Data< std::vector<Path> > m_paths;
};

int GeneratorMesh_Voronoi2Class = RegisterObject<GeneratorMesh_Voronoi2>("Generator/Mesh/Voronoi JC")
		.setDescription("Create a mesh from a Voronoi tessellation");


} // namespace Panda
