#ifndef TYPES_POINT_H
#define TYPES_POINT_H

#include <panda/core.h>

#ifndef PANDA_BUILD_CORE
#include <panda/types/Animation.h>
#include <panda/data/Data.h>
#endif

#include <cassert>
#include <cmath>
#include <limits>

namespace panda
{

namespace types
{

enum NoInit { NOINIT };

class PANDA_CORE_API Point
{
public:
	PReal x, y;

	static const int DIM = 2;

	Point();
	explicit Point(NoInit);
	Point(PReal x, PReal y);
	Point(const Point& p);
	explicit Point(const PReal* d);

	void set(PReal nx, PReal ny);
	void set(const Point& p);

	Point& operator=(const Point& p);

	PReal& operator[](int n);
	const PReal& operator[](int n) const;

	PReal* data();
	const PReal* data() const;

	Point operator+(const Point& p) const;
	Point operator-(const Point& p) const;
	Point& operator+=(const Point& p);
	Point& operator-=(const Point& p);

	Point operator*(PReal v) const;
	Point operator/(PReal v) const;
	Point& operator*=(PReal v);
	Point& operator/=(PReal v);

	friend Point operator*(PReal v, const Point& p);
	friend Point operator/(PReal v, const Point& p);

	// Negation
	Point operator-() const;

	Point linearProduct(const Point& p) const;
	Point linearDivision(const Point& p) const;

	// Dot product
	PReal operator*(const Point& p) const;
	PReal dot(const Point& p) const;

	// Z component of the cross product of two vectors on the XY plane
	PReal cross(const Point& p) const;

	PReal norm2() const;
	PReal norm() const;

	void normalizeWithNorm(PReal norm, PReal threshold=std::numeric_limits<PReal>::epsilon());
	Point normalizedWithNorm(PReal norm, PReal threshold=std::numeric_limits<PReal>::epsilon()) const;

	void normalize(PReal threshold=std::numeric_limits<PReal>::epsilon());
	Point normalized(PReal threshold=std::numeric_limits<PReal>::epsilon()) const;

	// True if norm() is very close to 1
	bool isNormalized(PReal threshold=std::numeric_limits<PReal>::epsilon()*(PReal)10);

	bool operator==(const Point& p) const;
	bool operator!=(const Point& p) const;

	static Point max();

	static Point zero();
	static Point one();

	static Point xAxis();
	static Point yAxis();
};

inline bool pFuzzyIsNull(double d)
{ return abs(d) <= 0.000000000001; }

inline bool pFuzzyIsNull(float f)
{ return abs(f) <= 0.00001f; }

inline Point::Point()
: x(0), y(0) { }

inline Point::Point(NoInit)
{ }

inline Point::Point(PReal x, PReal y)
: x(x), y(y) { }

inline Point::Point(const Point& p)
: x(p.x), y(p.y) { }

inline Point::Point(const PReal* d)
: x(d[0]), y(d[1]) { }

inline void Point::set(PReal nx, PReal ny)
{ x = nx; y = ny; }

inline void Point::set(const Point& p)
{ x = p.x; y = p.y; }

inline Point& Point::operator=(const Point& p)
{ x = p.x; y = p.y; return *this; }

inline PReal& Point::operator[](int n)
{ assert(n >= 0 && n <= 1); return (&x)[n]; }

inline const PReal& Point::operator[](int n) const
{ assert(n >= 0 && n <= 1); return (&x)[n]; }

inline PReal* Point::data()
{ return &x; }

inline const PReal* Point::data() const
{ return &x; }

inline Point Point::operator+(const Point& p) const
{ return Point(x + p.x, y + p.y); }

inline Point Point::operator-(const Point& p) const
{ return Point(x - p.x, y - p.y); }

inline Point& Point::operator+=(const Point& p)
{ x += p.x; y += p.y; return *this; }

inline Point& Point::operator-=(const Point& p)
{ x -= p.x; y -= p.y; return *this; }

inline Point Point::operator*(PReal v) const
{ return Point(x * v, y * v); }

inline Point Point::operator/(PReal v) const
{ return Point(x / v, y / v); }

inline Point& Point::operator*=(PReal v)
{ x *= v; y *= v; return *this; }

inline Point& Point::operator/=(PReal v)
{ x /= v; y /= v; return *this; }

inline Point operator*(PReal v, const Point& p)
{ return p * v; }

inline Point operator/(PReal v, const Point& p)
{ return p / v; }

inline Point Point::operator-() const
{ return Point(-x, -y); }

inline Point Point::linearProduct(const Point& p) const
{ return Point(x * p.x, y * p.y); }

inline Point Point::linearDivision(const Point& p) const
{ return Point(x / p.x, y / p.y); }

inline PReal Point::operator*(const Point& p) const
{ return x * p.x + y * p.y; }

inline PReal Point::dot(const Point& p) const
{ return x * p.x + y * p.y; }

inline PReal Point::cross(const Point& p) const
{ return x * p.y - y * p.x; }

inline PReal Point::norm2() const
{ return x * x + y * y; }

inline PReal Point::norm() const
{ return sqrt(norm2()); }

inline void Point::normalizeWithNorm(PReal norm, PReal threshold)
{
	if(norm > threshold) {
		PReal invN = 1 / norm;
		x *= invN;
		y *= invN;
	}
}

inline Point Point::normalizedWithNorm(PReal norm, PReal threshold) const
{
	if(norm > threshold) {
		PReal invN = 1 / norm;
		return Point(x * invN, y * invN);
	}
	return Point();
}

inline void Point::normalize(PReal threshold)
{ normalizeWithNorm(norm(), threshold); }

inline Point Point::normalized(PReal threshold) const
{ return normalizedWithNorm(norm(), threshold); }

inline bool Point::isNormalized(PReal threshold)
{ return abs(norm2() - static_cast<PReal>(1)) <= threshold; }

inline bool Point::operator==(const Point& p) const
{ return pFuzzyIsNull(x - p.x) && pFuzzyIsNull(y - p.y); }

inline bool Point::operator!=(const Point& p) const
{ return !pFuzzyIsNull(x - p.x) || !pFuzzyIsNull(y - p.y); }

inline Point Point::max()
{ return Point(std::numeric_limits<PReal>::max(), std::numeric_limits<PReal>::max()); }

inline Point Point::zero()
{ return Point(0, 0); }

inline Point Point::one()
{ return Point(1, 1); }

inline Point Point::xAxis()
{ return Point(1, 0); }

inline Point Point::yAxis()
{ return Point(0, 1); }

/// Rotate a point around the center, angle is given in radians
PANDA_CORE_API void rotate(Point& point, const Point& center, PReal angle);
PANDA_CORE_API Point rotated(const Point& point, const Point& center, PReal angle);

/// Converts a coordinate from rectangular (Cartesian) coordinates to polar coordinates of the form (radius, theta)
PANDA_CORE_API Point toPolar(Point car);

/// Converts a coordinate from polar coordinates of the form (radius, theta) to rectangular coordinates
PANDA_CORE_API Point fromPolar(Point pol);

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API std::vector<Point>;
extern template class PANDA_CORE_API Data< Point >;
extern template class PANDA_CORE_API Data< std::vector<Point> >;
extern template class PANDA_CORE_API Animation<Point>;
extern template class PANDA_CORE_API Data< Animation<Point> >;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_POINT_H
