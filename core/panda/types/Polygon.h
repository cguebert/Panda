#ifndef TYPES_POLYGON_H
#define TYPES_POLYGON_H

#include <panda/core.h>
#include <panda/types/Path.h>

namespace panda
{

namespace types
{

class PANDA_CORE_API Polygon
{
public:
	void clear();

	bool operator==(const Polygon& p) const;
	bool operator!=(const Polygon& p) const;

	Path contour;
	std::vector<Path> holes;
};

PANDA_CORE_API void translate(Polygon& poly, const Point& delta);
PANDA_CORE_API Polygon translated(const Polygon& poly, const Point& delta);

PANDA_CORE_API void scale(Polygon& poly, PReal scale);
PANDA_CORE_API Polygon scaled(const Polygon& poly, PReal scale);

/// Rotate every point of the paths around the center, angle is given in radians
PANDA_CORE_API void rotate(Polygon& poly, const Point& center, PReal angle);
PANDA_CORE_API Polygon rotated(const Polygon& poly, const Point& center, PReal angle);

inline bool Polygon::operator==(const Polygon& p) const
{ return contour == p.contour && holes == p.holes; }

inline bool Polygon::operator!=(const Polygon& p) const
{ return contour != p.contour || holes != p.holes; }

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data< Polygon >;
extern template class PANDA_CORE_API Data< std::vector<Polygon> >;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_POLYGON_H
