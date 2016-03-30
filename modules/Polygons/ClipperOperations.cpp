#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Polygon.h>

#include "ClipperUtils.h"

namespace panda {

using types::Point;
using types::Path;
using types::Polygon;

class ClipperOperation_Simplify : public PandaObject
{
public:
	PANDA_CLASS(ClipperOperation_Simplify, PandaObject)

	ClipperOperation_Simplify(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input polygon"))
		, m_output(initData("output", "Simplified polygon"))
		, m_fillType(initData(0, "fill type", "Rule for the filling of the polygons"))
	{
		addInput(m_input);
		addInput(m_fillType);
		addOutput(m_output);

		m_fillType.setWidget("enum");
		m_fillType.setWidgetData("Even-Odd;Non-Zero;Positive;Negative");
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto acc = m_output.getAccessor();
		acc.clear();
		auto& outPolys = acc.wref();

		ClipperLib::PolyFillType fillType = ClipperLib::pftEvenOdd;
		auto fillVal = m_fillType.getValue();
		switch (fillVal)
		{
		default:
		case 0: fillType = ClipperLib::pftEvenOdd; break;
		case 1: fillType = ClipperLib::pftNonZero; break;
		case 2: fillType = ClipperLib::pftPositive; break;
		case 3: fillType = ClipperLib::pftNegative; break;
		}

		for (const auto& poly : input)
		{
			auto cPaths = polyToClipperPaths(poly);
			ClipperLib::SimplifyPolygons(cPaths, fillType);
			auto polys = clipperPathsToPolys(cPaths);
			outPolys.insert(outPolys.end(), polys.begin(), polys.end());
		}
	}

protected:
	Data< std::vector<Polygon> > m_input, m_output;
	Data< int > m_fillType;
};

int ClipperOperation_SimplifyClass = RegisterObject<ClipperOperation_Simplify>("Math/Polygon/Simplify")
	.setName("Simplify polygon")
	.setDescription("Removes self-intersections from the input polygon");

//****************************************************************************//

class ClipperOperation_Clean : public PandaObject
{
public:
	PANDA_CLASS(ClipperOperation_Clean, PandaObject)

	ClipperOperation_Clean(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input polygon"))
		, m_output(initData("output", "Cleaned polygon"))
		, m_distance(initData(1.415, "distance", "Distance below which adjacents vertices are removed"))
	{
		addInput(m_input);
		addInput(m_distance);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto acc = m_output.getAccessor();
		acc.clear();
		auto& outPolys = acc.wref();

		auto dist = m_distance.getValue() * pandaToClipperFactor;

		for (const auto& poly : input)
		{
			auto cPaths = polyToClipperPaths(poly);
			ClipperLib::CleanPolygons(cPaths, dist);
			auto polys = clipperPathsToPolys(cPaths);
			outPolys.insert(outPolys.end(), polys.begin(), polys.end());
		}
	}

protected:
	Data< std::vector<Polygon> > m_input, m_output;
	Data< float > m_distance;
};

int ClipperOperation_CleanClass = RegisterObject<ClipperOperation_Clean>("Math/Polygon/Clean")
	.setName("Clean polygon")
	.setDescription("Removes duplicate vertices and colinear edges from the input polygon");

//****************************************************************************//

class ClipperOperation_OffsetPolygon : public PandaObject
{
public:
	PANDA_CLASS(ClipperOperation_OffsetPolygon, PandaObject)

	ClipperOperation_OffsetPolygon(PandaDocument* doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input polygon"))
		, m_output(initData("output", "Offsetted polygon"))
		, m_distance(initData(0, "distance", "Offset"))
		, m_joinType(initData(0, "join type", "Type for the joins"))
		, m_endType(initData(0, "end type", "Type for the end"))
		, m_arcTolerance(initData(0.25, "arc tolerance", "Acceptable imprecision when approximating arcs"))
		, m_together(initData(1, "together", "Offset all polygons as one, or separately"))
	{
		addInput(m_input);
		addInput(m_distance);
		addInput(m_joinType);
		addInput(m_endType);
		addInput(m_arcTolerance);
		addInput(m_together);

		addOutput(m_output);

		m_joinType.setWidget("enum");
		m_joinType.setWidgetData("Square;Round;Mitter");

		m_endType.setWidget("enum");
		m_endType.setWidgetData("Close polygon;Close line;Open butt;Open square;Open round");

		m_together.setWidget("checkbox");
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto acc = m_output.getAccessor();
		acc.clear();
		auto& outPolys = acc.wref();

		auto dist = m_distance.getValue() * pandaToClipperFactor;
		float arcTolerance = m_arcTolerance.getValue() * pandaToClipperFactor;
		
		int joinVal = m_joinType.getValue();
		ClipperLib::JoinType joinType = ClipperLib::jtSquare;
		if (joinVal == 1)
			joinType = ClipperLib::jtRound;
		else if (joinVal == 2)
			joinType = ClipperLib::jtMiter;

		int endVal = m_endType.getValue();
		ClipperLib::EndType endType = ClipperLib::etClosedPolygon;
		switch (endVal)
		{
		default:
		case 0: endType = ClipperLib::etClosedPolygon; break;
		case 1: endType = ClipperLib::etClosedLine; break;
		case 2: endType = ClipperLib::etOpenButt; break;
		case 3: endType = ClipperLib::etOpenSquare; break;
		case 4: endType = ClipperLib::etOpenRound; break;
		}

		if (m_together.getValue() != 0)
		{
			ClipperLib::ClipperOffset co(2.0, arcTolerance);
			for (const auto& poly : input)
				co.AddPaths(polyToClipperPaths(poly), joinType, endType);

			ClipperLib::Paths result;
			co.Execute(result, dist);
			auto polys = clipperPathsToPolys(result);
			outPolys.insert(outPolys.end(), polys.begin(), polys.end());
		}
		else
		{
			for (const auto& poly : input)
			{
				ClipperLib::ClipperOffset co(2.0, arcTolerance);
				co.AddPaths(polyToClipperPaths(poly), joinType, endType);

				ClipperLib::Paths result;
				co.Execute(result, dist);
				auto polys = clipperPathsToPolys(result);
				outPolys.insert(outPolys.end(), polys.begin(), polys.end());
			}
		}
	}

protected:
	Data< std::vector<Polygon> > m_input, m_output;
	Data< float > m_distance;
	Data< int > m_joinType, m_endType;
	Data< float > m_arcTolerance;
	Data< int > m_together;
};

int ClipperOperation_OffsetPolygonClass = RegisterObject<ClipperOperation_OffsetPolygon>("Math/Polygon/Offset polygon")
	.setDescription("Offset (inflate or deflate) polygons");

//****************************************************************************//

class ClipperOperation_MinkowskiSum : public PandaObject
{
public:
	PANDA_CLASS(ClipperOperation_MinkowskiSum, PandaObject)

		ClipperOperation_MinkowskiSum(PandaDocument* doc)
		: PandaObject(doc)
		, m_output(initData("output", "Extruded polygon"))
		, m_input(initData("input", "Input polygon"))
		, m_path(initData("path", "Path along which to extrude the polygon"))
	{
		addInput(m_input);
		addInput(m_path);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& paths = m_path.getValue();
		auto acc = m_output.getAccessor();
		acc.clear();
		auto& outPolys = acc.wref();

		int nbPolys = input.size();
		int nbPaths = paths.size();

		if (nbPolys && nbPaths)
		{
			if (nbPolys > 1 && nbPaths > 1 && nbPolys != nbPaths)
				nbPolys = nbPaths = 1;
			int nb = std::max(nbPolys, nbPaths);

			for (int i = 0; i < nb; ++i)
			{
				const auto& pattern = input[i % nbPolys];
				const auto& path = paths[i % nbPaths];
				bool closed = false;
				if (path.points.size() > 1 && path.points.front() == path.points.back())
					closed = true;
				auto cPattern = pathToClipperPath(pattern);
				auto cPath = pathToClipperPath(path);
				ClipperLib::Paths result;
				ClipperLib::MinkowskiSum(cPattern, cPath, result, closed);

				auto polys = clipperPathsToPolys(result);
				outPolys.insert(outPolys.end(), polys.begin(), polys.end());
			}
		}
	}

protected:
	Data< std::vector<Polygon> > m_output;
	Data< std::vector<Path> > m_input, m_path;
};

int ClipperOperation_MinkowskiSumClass = RegisterObject<ClipperOperation_MinkowskiSum>("Math/Polygon/Extrude polygon")
	.setDescription("Extrude a polygon along a path");

} // namespace Panda
