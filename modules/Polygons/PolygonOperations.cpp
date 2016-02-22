#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Polygon.h>

#include <cmath>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(disable : 4244) /* conversion from 'type1' to 'type2', possible loss of data */
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace panda {

using types::Point;
using types::Path;
using types::Polygon;

class PolygonOperation_Difference : public PandaObject
{
public:
	PANDA_CLASS(PolygonOperation_Difference, PandaObject)

	PolygonOperation_Difference(PandaDocument *doc)
		: PandaObject(doc)
		, m_inputA(initData("input 1", "First polygon"))
		, m_inputB(initData("input 2", "Second polygon"))
		, m_output(initData("output", "Result of the operation"))
	{
		addInput(m_inputA);
		addInput(m_inputB);

		addOutput(m_output);
	}

	void update()
	{
		const Polygon& inputA = m_inputA.getValue();
		const Polygon& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if(inputA.contour.empty() || inputB.contour.empty())
			return;

		typedef boost::geometry::model::d2::point_xy<float> BGPoint;
		typedef boost::geometry::model::polygon<BGPoint> BGPolygon;
		BGPolygon pA, pB;
		{
			std::vector<BGPoint> pts;
			for(const Point& pt : inputA.contour)
				pts.emplace_back(pt.x, pt.y);
			boost::geometry::append(pA, pts);
		}

		{
			std::vector<BGPoint> pts;
			for(const Point& pt : inputB.contour)
				pts.emplace_back(pt.x, pt.y);
			boost::geometry::append(pB, pts);
		}

		std::vector<BGPolygon> result;
		boost::geometry::difference(pA, pB, result);

		for(const BGPolygon& rpoly : result)
		{
			Polygon poly;
			Path path;
			path.reserve((int)rpoly.outer().size());
			for(const auto& pt : rpoly.outer())
				path.push_back(Point(pt.x(), pt.y()));
			poly.contour = path;

			for(const auto& inner : rpoly.inners())
			{
				Path path;
				path.reserve((int)inner.size());
				for(const auto& pt : inner)
					path.push_back(Point(pt.x(), pt.y()));
				poly.holes.push_back(path);
			}

			output.push_back(poly);
		}
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< std::vector<Polygon> > m_output;
};

int PolygonOperation_DifferenceClass = RegisterObject<PolygonOperation_Difference>("Math/Polygon/Difference").setDescription("Compute the difference of two polygons");

//****************************************************************************//

class PolygonOperation_Union : public PandaObject
{
public:
	PANDA_CLASS(PolygonOperation_Union, PandaObject)

	PolygonOperation_Union(PandaDocument *doc)
		: PandaObject(doc)
		, m_inputA(initData("input 1", "First polygon"))
		, m_inputB(initData("input 2", "Second polygon"))
		, m_output(initData("output", "Result of the operation"))
	{
		addInput(m_inputA);
		addInput(m_inputB);

		addOutput(m_output);
	}

	void update()
	{
		const Polygon& inputA = m_inputA.getValue();
		const Polygon& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if(inputA.contour.empty() || inputB.contour.empty())
			return;

		typedef boost::geometry::model::d2::point_xy<float> BGPoint;
		typedef boost::geometry::model::polygon<BGPoint> BGPolygon;
		BGPolygon pA, pB;
		{
			std::vector<BGPoint> pts;
			for(const Point& pt : inputA.contour)
				pts.emplace_back(pt.x, pt.y);
			boost::geometry::append(pA, pts);
		}

		{
			std::vector<BGPoint> pts;
			for(const Point& pt : inputB.contour)
				pts.emplace_back(pt.x, pt.y);
			boost::geometry::append(pB, pts);
		}

		std::vector<BGPolygon> result;
		boost::geometry::union_(pA, pB, result);

		for(const BGPolygon& rpoly : result)
		{
			Polygon poly;
			Path path;
			path.reserve((int)rpoly.outer().size());
			for(const auto& pt : rpoly.outer())
				path.push_back(Point(pt.x(), pt.y()));
			poly.contour = path;

			for(const auto& inner : rpoly.inners())
			{
				Path path;
				path.reserve((int)inner.size());
				for(const auto& pt : inner)
					path.push_back(Point(pt.x(), pt.y()));
				poly.holes.push_back(path);
			}

			output.push_back(poly);
		}
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< std::vector<Polygon> > m_output;
};

int PolygonOperation_UnionClass = RegisterObject<PolygonOperation_Union>("Math/Polygon/Union").setDescription("Compute the union of two polygons");

} // namespace Panda


