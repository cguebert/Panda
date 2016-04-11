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

namespace
{
	using BGPoint = boost::geometry::model::d2::point_xy<float>;
	using BGPolygon = boost::geometry::model::polygon<BGPoint>;
	using BGRing = typename BGPolygon::ring_type;
	using BGPolygonList = std::vector<BGPolygon>;

	inline std::vector<BGPoint> convert(const panda::types::Path& path)
	{
		std::vector<BGPoint> pts;
		for (const auto& pt : path.points)
			pts.emplace_back(pt.x, pt.y);
		return pts;
	}

	inline BGPolygon convert(const panda::types::Polygon& inputPoly)
	{
		BGPolygon poly;
		boost::geometry::append(poly, convert(inputPoly.contour));

		int nbHoles = inputPoly.holes.size();
		poly.inners().resize(nbHoles);
		for (int i = 0; i < nbHoles; ++i)
			boost::geometry::append(poly.inners()[0], convert(inputPoly.holes[i]));
		 boost::geometry::correct(poly);
		return poly;
	}

	inline panda::types::Path convert(const BGRing& ring)
	{
		panda::types::Path path;
		path.points.reserve((int)ring.size());
		for(const auto& pt : ring)
			path.points.emplace_back(pt.x(), pt.y());
		return path;
	}

	inline panda::types::Polygon convert(const BGPolygon& inputPoly)
	{
		panda::types::Polygon poly;
		poly.contour = convert(inputPoly.outer());

		for(const auto& inner : inputPoly.inners())
			poly.holes.push_back(convert(inner));

		return poly;
	}

	inline std::vector<panda::types::Polygon> convert(const BGPolygonList& inputPolys)
	{
		std::vector<panda::types::Polygon> output;
		for(const BGPolygon& inPoly : inputPolys)
			output.push_back(convert(inPoly));

		return output;
	}
}

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
		const auto& inputA = m_inputA.getValue();
		const auto& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if (inputA.contour.points.empty() || inputB.contour.points.empty())
			return;

		BGPolygon pA = convert(inputA), pB = convert(inputB);
		BGPolygonList result;
		boost::geometry::difference(pA, pB, result);
		output = convert(result);
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< std::vector<Polygon> > m_output;
};

int PolygonOperation_DifferenceClass = RegisterObject<PolygonOperation_Difference>("Math/Polygon/Difference")
	.setName("Polygons difference").setDescription("Compute the difference of two polygons");

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
		const auto& inputA = m_inputA.getValue();
		const auto& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if (inputA.contour.points.empty() || inputB.contour.points.empty())
			return;

		BGPolygon pA = convert(inputA), pB = convert(inputB);
		BGPolygonList result;
		boost::geometry::union_(pA, pB, result);
		output = convert(result);
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< std::vector<Polygon> > m_output;
};

int PolygonOperation_UnionClass = RegisterObject<PolygonOperation_Union>("Math/Polygon/Union")
	.setName("Polygons union").setDescription("Compute the union of two polygons");

//****************************************************************************//

class PolygonOperation_Intersection : public PandaObject
{
public:
	PANDA_CLASS(PolygonOperation_Intersection, PandaObject)

	PolygonOperation_Intersection(PandaDocument *doc)
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
		const auto& inputA = m_inputA.getValue();
		const auto& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if (inputA.contour.points.empty() || inputB.contour.points.empty())
			return;

		BGPolygon pA = convert(inputA), pB = convert(inputB);
		BGPolygonList result;
		boost::geometry::intersection(pA, pB, result);
		output = convert(result);
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< std::vector<Polygon> > m_output;
};

int PolygonOperation_IntersectionClass = RegisterObject<PolygonOperation_Intersection>("Math/Polygon/Intersection")
	.setName("Polygons intersection").setDescription("Compute the intersection of two polygons");

//****************************************************************************//

class PolygonOperation_Xor : public PandaObject
{
public:
	PANDA_CLASS(PolygonOperation_Xor, PandaObject)

	PolygonOperation_Xor(PandaDocument *doc)
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
		const auto& inputA = m_inputA.getValue();
		const auto& inputB = m_inputB.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		if (inputA.contour.points.empty() || inputB.contour.points.empty())
			return;

		BGPolygon pA = convert(inputA), pB = convert(inputB);
		BGPolygonList result;
		boost::geometry::sym_difference(pA, pB, result);
		output = convert(result);
	}

protected:
	Data< Polygon > m_inputA, m_inputB;
	Data< std::vector<Polygon> > m_output;
};

int PolygonOperation_XorClass = RegisterObject<PolygonOperation_Xor>("Math/Polygon/Xor")
	.setName("Polygons Xor").setDescription("Compute the symmetric difference of two polygons");

//****************************************************************************//

class PolygonOperation_ConvexHull : public PandaObject
{
public:
	PANDA_CLASS(PolygonOperation_ConvexHull, PandaObject)

	PolygonOperation_ConvexHull(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input polygon"))
		, m_output(initData("output", "Convex hull of the polygon"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();
		output.clear();

		for (const auto& inPoly : input)
		{
			Polygon outPoly;
			if (!inPoly.contour.points.empty())
			{
				BGPolygon bgIn = convert(inPoly);
				BGPolygon bgOut;
				boost::geometry::convex_hull(bgIn, bgOut);
				outPoly = convert(bgOut);
			}
			output.push_back(outPoly);
		}
	}

protected:
	Data< std::vector<Polygon> > m_input, m_output;
};

int PolygonOperation_ConvexHullClass = RegisterObject<PolygonOperation_ConvexHull>("Math/Polygon/Convex hull")
	.setDescription("Compute the onvex hull of a polygon");

} // namespace Panda


