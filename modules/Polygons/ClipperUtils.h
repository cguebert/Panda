#pragma once

#include <panda/types/Polygon.h>

#include <modules/Polygons/libs/clipper/clipper.hpp>

namespace panda
{
	const float pandaToClipperFactor = 100.f;
	const float clipperToPandaFactor = 1.f / pandaToClipperFactor;

	inline ClipperLib::IntPoint convert(const panda::types::Point& pt)
	{ return ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(pt.x * pandaToClipperFactor), 
		static_cast<ClipperLib::cInt>(pt.y * pandaToClipperFactor)); }

	inline panda::types::Point convert(const ClipperLib::IntPoint& pt)
	{ return panda::types::Point(static_cast<float>(pt.X * clipperToPandaFactor),
		static_cast<float>(pt.Y * clipperToPandaFactor)); }

	ClipperLib::Path pathToClipperPath(const panda::types::Path& path);
	panda::types::Path clipperPathToPath(const ClipperLib::Path& path);

	ClipperLib::Paths polyToClipperPaths(const panda::types::Polygon& poly);
	std::vector<panda::types::Polygon> clipperPathsToPolys(const ClipperLib::Paths& paths);

}
