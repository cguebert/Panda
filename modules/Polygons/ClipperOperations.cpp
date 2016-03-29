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

	ClipperOperation_Simplify(PandaDocument *doc)
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

	ClipperOperation_Clean(PandaDocument *doc)
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
	.setDescription("Removes duplicate vertices and colinear edges from the input polygon");} // namespace Panda


