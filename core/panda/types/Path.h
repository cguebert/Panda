#ifndef TYPES_PATH_H
#define TYPES_PATH_H

#include <panda/core.h>
#include <panda/types/Point.h>

#include <vector>

namespace panda
{

namespace types
{

class PANDA_CORE_API Path
{
public:
	std::vector<Point> points;

	Path() = default;
	Path(const std::vector<Point>& v);

	Path& operator=(const std::vector<Point>& v);
	Path& operator=(const Path& v);

	// Operation on all points
	Path operator+(const Point& p) const;
	Path operator-(const Point& p) const;
	Path& operator+=(const Point& p);
	Path& operator-=(const Point& p);

	Path operator*(float v) const;
	Path operator/(float v) const;
	Path& operator*=(float v);
	Path& operator/=(float v);

	friend Path operator*(float v, const Path& p);

	Path linearProduct(const Point& p) const;
	Path linearDivision(const Point& p) const;

	Path reversed() const;
	void reverse();

	bool operator==(const Path& p) const;
	bool operator!=(const Path& p) const;
};

/// Rotate every point of the path around the center, angle is given in radians
PANDA_CORE_API void rotate(Path& path, const Point& center, float angle);
PANDA_CORE_API Path rotated(const Path& path, const Point& center, float angle);

PANDA_CORE_API float areaOfPolygon(const Path& poly);
PANDA_CORE_API types::Point centroidOfPolygon(const Path& poly);
PANDA_CORE_API bool polygonContainsPoint(const Path &poly, types::Point pt);
PANDA_CORE_API void reorientPolygon(Path& poly);

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<Path>;
extern template class PANDA_CORE_API Data<std::vector<Path>>;
#endif

inline Path::Path(const std::vector<Point>& v)
: points(v) {}

inline Path& Path::operator=(const std::vector<Point> &v)
{ points = v; return *this; }

inline Path& Path::operator=(const Path &v)
{ points = v.points; return *this; }

inline Path& Path::operator+=(const Point& p)
{ for (auto& pt : points) pt += p; return *this; }

inline Path& Path::operator-=(const Point& p)
{ for (auto& pt : points) pt -= p; return *this; }

inline Path& Path::operator*=(float v)
{ for (auto& pt : points) pt *= v; return *this; }

inline Path& Path::operator/=(float v)
{ for (auto& pt : points) pt /= v; return *this; }

inline bool Path::operator==(const Path& p) const
{ return points == p.points; }

inline bool Path::operator!=(const Path& p) const
{ return points != p.points; }

} // namespace types

} // namespace panda

#endif // TYPES_PATH_H
