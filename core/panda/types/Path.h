#ifndef TYPES_PATH_H
#define TYPES_PATH_H

#include <panda/core.h>
#include <panda/types/Point.h>

#include <QVector>

namespace panda
{

namespace types
{

class PANDA_CORE_API Path : public QVector<Point>
{
public:
	Path& operator=(const QVector<Point>& v);

	// Operation on all points
	Path operator+(const Point& p) const;
	Path operator-(const Point& p) const;
	Path& operator+=(const Point& p);
	Path& operator-=(const Point& p);

	Path operator*(PReal v) const;
	Path operator/(PReal v) const;
	Path& operator*=(PReal v);
	Path& operator/=(PReal v);

	friend Path operator*(PReal v, const Path& p);
	friend Path operator/(PReal v, const Path& p);

	Path linearProduct(const Point& p) const;
	Path linearDivision(const Point& p) const;

	Path reversed() const;
	void reverse();
};

/// Rotate every point of the path around the center, angle is given in radians
PANDA_CORE_API void rotate(Path& path, const Point& center, PReal angle);
PANDA_CORE_API Path rotated(const Path& path, const Point& center, PReal angle);

PANDA_CORE_API PReal areaOfPolygon(const Path& poly);
PANDA_CORE_API types::Point centroidOfPolygon(const Path& poly);
PANDA_CORE_API bool polygonContainsPoint(const Path &poly, types::Point pt);
PANDA_CORE_API void reorientPolygon(Path& poly);

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data< Path >;
extern template class PANDA_CORE_API Data< QVector<Path> >;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_PATH_H
