#include "ClipperUtils.h"

namespace panda
{

	ClipperLib::Path pathToClipperPath(const panda::types::Path& path)
	{
		ClipperLib::Path out;
		auto maxVal = std::numeric_limits<ClipperLib::cInt>::max();
		ClipperLib::IntPoint prevPt(maxVal, maxVal);
		for (const auto& pt : path.points)
		{
			auto newPt = convert(pt);
			if (newPt == prevPt)
				continue;
			out.push_back(newPt);
			prevPt = newPt;
		}

		if (path.points.size() > 1 && path.points.back() == path.points.front())
			out.pop_back();
		return out;
	}

	panda::types::Path clipperPathToPath(const ClipperLib::Path& path)
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
		if (!Orientation(contour)) // We want the orientation to be CW
			ReversePath(contour);
		paths.push_back(contour);

		for (const auto& hole : poly.holes)
		{
			auto path = pathToClipperPath(hole);
			if (path.size() < 3)
				continue;

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
