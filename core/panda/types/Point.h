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
	float x, y;

	static const int DIM = 2;

	Point();
	explicit Point(NoInit);
	Point(float x, float y);
	Point(const Point& p);
	explicit Point(const float* d);

	void set(float nx, float ny);
	void set(const Point& p);

	Point& operator=(const Point& p);

	float& operator[](int n);
	const float& operator[](int n) const;

	float* data();
	const float* data() const;

	Point operator+(const Point& p) const;
	Point operator-(const Point& p) const;
	Point& operator+=(const Point& p);
	Point& operator-=(const Point& p);

	Point operator*(float v) const;
	Point operator/(float v) const;
	Point& operator*=(float v);
	Point& operator/=(float v);

	friend Point operator*(float v, const Point& p);

	// Negation
	Point operator-() const;

	Point linearProduct(const Point& p) const;
	Point linearDivision(const Point& p) const;

	// Dot product
	float operator*(const Point& p) const;
	float dot(const Point& p) const;

	// Z component of the cross product of two vectors on the XY plane
	float cross(const Point& p) const;

	float norm2() const;
	float norm() const;

	void normalizeWithNorm(float norm, float threshold=std::numeric_limits<float>::epsilon());
	Point normalizedWithNorm(float norm, float threshold=std::numeric_limits<float>::epsilon()) const;

	void normalize(float threshold=std::numeric_limits<float>::epsilon());
	Point normalized(float threshold=std::numeric_limits<float>::epsilon()) const;

	// True if norm() is very close to 1
	bool isNormalized(float threshold=std::numeric_limits<float>::epsilon()*(float)10);

	bool operator==(const Point& p) const;
	bool operator!=(const Point& p) const;

	static Point max();

	static Point zero();
	static Point one();

	static Point xAxis();
	static Point yAxis();
};

inline bool pFuzzyIsNull(float f)
{ return abs(f) <= 0.00001f; }

inline Point::Point()
: x(0), y(0) { }

inline Point::Point(NoInit)
{ }

inline Point::Point(float x, float y)
: x(x), y(y) { }

inline Point::Point(const Point& p)
: x(p.x), y(p.y) { }

inline Point::Point(const float* d)
: x(d[0]), y(d[1]) { }

inline void Point::set(float nx, float ny)
{ x = nx; y = ny; }

inline void Point::set(const Point& p)
{ x = p.x; y = p.y; }

inline Point& Point::operator=(const Point& p)
{ x = p.x; y = p.y; return *this; }

inline float& Point::operator[](int n)
{ assert(n >= 0 && n <= 1); return (&x)[n]; }

inline const float& Point::operator[](int n) const
{ assert(n >= 0 && n <= 1); return (&x)[n]; }

inline float* Point::data()
{ return &x; }

inline const float* Point::data() const
{ return &x; }

inline Point Point::operator+(const Point& p) const
{ return Point(x + p.x, y + p.y); }

inline Point Point::operator-(const Point& p) const
{ return Point(x - p.x, y - p.y); }

inline Point& Point::operator+=(const Point& p)
{ x += p.x; y += p.y; return *this; }

inline Point& Point::operator-=(const Point& p)
{ x -= p.x; y -= p.y; return *this; }

inline Point Point::operator*(float v) const
{ return Point(x * v, y * v); }

inline Point Point::operator/(float v) const
{ return Point(x / v, y / v); }

inline Point& Point::operator*=(float v)
{ x *= v; y *= v; return *this; }

inline Point& Point::operator/=(float v)
{ x /= v; y /= v; return *this; }

inline Point operator*(float v, const Point& p)
{ return p * v; }

inline Point Point::operator-() const
{ return Point(-x, -y); }

inline Point Point::linearProduct(const Point& p) const
{ return Point(x * p.x, y * p.y); }

inline Point Point::linearDivision(const Point& p) const
{ return Point(x / p.x, y / p.y); }

inline float Point::operator*(const Point& p) const
{ return x * p.x + y * p.y; }

inline float Point::dot(const Point& p) const
{ return x * p.x + y * p.y; }

inline float Point::cross(const Point& p) const
{ return x * p.y - y * p.x; }

inline float Point::norm2() const
{ return x * x + y * y; }

inline float Point::norm() const
{ return sqrt(norm2()); }

inline void Point::normalizeWithNorm(float norm, float threshold)
{
	if(norm > threshold) {
		float invN = 1 / norm;
		x *= invN;
		y *= invN;
	}
}

inline Point Point::normalizedWithNorm(float norm, float threshold) const
{
	if(norm > threshold) {
		float invN = 1 / norm;
		return Point(x * invN, y * invN);
	}
	return Point();
}

inline void Point::normalize(float threshold)
{ normalizeWithNorm(norm(), threshold); }

inline Point Point::normalized(float threshold) const
{ return normalizedWithNorm(norm(), threshold); }

inline bool Point::isNormalized(float threshold)
{ return abs(norm2() - 1.f) <= threshold; }

inline bool Point::operator==(const Point& p) const
{ return pFuzzyIsNull(x - p.x) && pFuzzyIsNull(y - p.y); }

inline bool Point::operator!=(const Point& p) const
{ return !pFuzzyIsNull(x - p.x) || !pFuzzyIsNull(y - p.y); }

inline Point Point::max()
{ return Point(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()); }

inline Point Point::zero()
{ return Point(0, 0); }

inline Point Point::one()
{ return Point(1, 1); }

inline Point Point::xAxis()
{ return Point(1, 0); }

inline Point Point::yAxis()
{ return Point(0, 1); }

inline bool operator<(const Point& lhs, const Point& rhs)
{
	if (lhs.x == rhs.x)
		return lhs.y < rhs.y;
	return lhs.x < rhs.x;
}

/// Rotate a point around the center, angle is given in radians
PANDA_CORE_API void rotate(Point& point, const Point& center, float angle);
PANDA_CORE_API Point rotated(const Point& point, const Point& center, float angle);

/// Converts a coordinate from rectangular (Cartesian) coordinates to polar coordinates of the form (radius, theta)
PANDA_CORE_API Point toPolar(Point car);

/// Converts a coordinate from polar coordinates of the form (radius, theta) to rectangular coordinates
PANDA_CORE_API Point fromPolar(Point pol);

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<Point>;
extern template class PANDA_CORE_API Data<std::vector<Point>>;
extern template class PANDA_CORE_API Animation<Point>;
extern template class PANDA_CORE_API Data<Animation<Point>>;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_POINT_H
