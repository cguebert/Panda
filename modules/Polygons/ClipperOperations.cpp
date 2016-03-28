#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Polygon.h>

#include <modules/Polygons/libs/clipper/clipper.hpp>

namespace
{
	const float pandaToClipperFactor = 100.f;
	const float clipperToPandaFactor = 1.f / pandaToClipperFactor;

	inline ClipperLib::IntPoint convert(const panda::types::Point& pt)
	{ return ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(pt.x * pandaToClipperFactor), 
		static_cast<ClipperLib::cInt>(pt.y * pandaToClipperFactor)); }

	inline panda::types::Point convert(const ClipperLib::IntPoint& pt)
	{ return panda::types::Point(static_cast<float>(pt.X * clipperToPandaFactor),
		static_cast<float>(pt.Y * clipperToPandaFactor)); }

	inline ClipperLib::Path pathToClipperPath(const panda::types::Path& path)
	{
		ClipperLib::Path out;
		if (path.points.empty())
			return out;
		for (const auto& pt : path.points)
			out.push_back(convert(pt));
		if (path.points.back() == path.points.front())
			out.pop_back();
		return out;
	}

	inline panda::types::Path clipperPathToPath(const ClipperLib::Path& path)
	{
		panda::types::Path out;
		if (path.empty())
			return out;
		for (const auto& pt : path)
			out.points.push_back(convert(pt));
		if (out.points.front() != out.points.back())
			out.points.push_back(out.points.front());
		return out;
	}

	ClipperLib::Paths polyToClipperPaths(const panda::types::Polygon& poly)
	{
		ClipperLib::Paths paths;
		auto contour = pathToClipperPath(poly.contour);
		paths.push_back(contour);
		if (!Orientation(contour)) // We want the orientation to be CW
			ReversePath(contour);

		for (const auto& hole : poly.holes)
		{
			auto path = pathToClipperPath(hole);

			// The orientation of holes must be opposite that of outer polygons.
			if (Orientation(path))
				ReversePath(path);
			paths.push_back(path);
		}

		return paths;
	}

	std::vector<panda::types::Polygon> clipperPathsToPolys(const ClipperLib::Paths& paths)
	{
		std::vector<panda::types::Polygon> polys;
		std::vector<panda::types::Path> holes;
		for (const auto& path : paths)
		{
			auto pPath = clipperPathToPath(path);
			if (Orientation(path))
			{
				panda::types::Polygon outPoly;
				outPoly.contour = std::move(pPath);
				polys.push_back(std::move(outPoly));
			}
			else if (!path.empty())
			{
				if (!polys.empty() && polygonContainsPoint(polys.back().contour, pPath.points[0]))
					polys.back().holes.push_back(std::move(pPath));
				else
					holes.push_back(std::move(pPath));
			}
		}

		// Find the correct polygon to put holes in
		std::vector<panda::types::Path> orphans;
		for (auto& hole : holes)
		{
			bool found = false;
			for (auto& poly : polys)
			{
				if (polygonContainsPoint(poly.contour, hole.points[0]))
				{
					poly.holes.push_back(std::move(hole));
					found = true;
					break;
				}
			}

			if (!found)
				orphans.push_back(std::move(hole));
		}

		// If we really didn't find where to place that path, we create a new polygon for it
		for (auto& orphan : orphans)
		{
			panda::types::Polygon poly;
			poly.contour = std::move(orphan);
			polys.push_back(std::move(poly));
		}
		return polys;
	}
}

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
		if (fillVal == 1)
			fillType = ClipperLib::pftNonZero;

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


