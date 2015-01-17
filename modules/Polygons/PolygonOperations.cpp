#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Polygon.h>
#include <QVector>

#include <cmath>
#include <algorithm>
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
		, m_inputA(initData(&m_inputA, "input 1", "First polygon"))
		, m_inputB(initData(&m_inputB, "input 2", "Second polygon"))
		, m_output(initData(&m_output, "output", "Result of the operation"))
	{
		addInput(&m_inputA);
		addInput(&m_inputB);

		addOutput(&m_output);
	}

	void update()
	{
		const Polygon& inputA = m_inputA.getValue();
		const Polygon& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if(inputA.contour.empty() || inputB.contour.empty())
		{
			cleanDirty();
			return;
		}

		typedef boost::geometry::model::d2::point_xy<PReal> BGPoint;
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

		cleanDirty();
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< QVector<Polygon> > m_output;
};

int PolygonOperation_DifferenceClass = RegisterObject<PolygonOperation_Difference>("Math/Polygon/Difference").setDescription("Compute the difference of two polygons");

//****************************************************************************//



} // namespace Panda


