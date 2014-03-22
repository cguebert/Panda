#ifndef TYPES_POINT_H
#define TYPES_POINT_H
#pragma once

#include <panda/helper/system/Config.h>

#include <cassert>
#include <cmath>
#include <limits>

#define EQUALITY_THRESHOLD 1e-6

namespace panda
{

namespace types
{

inline bool pFuzzyIsNull(double d)
{ return abs(d) <= 0.000000000001; }

inline bool pFuzzyIsNull(float f)
{ return abs(f) <= 0.00001f; }

enum NoInit { NOINIT };

class Point
{
public:
	PReal x, y;

	static const int DIM = 2;

	Point() : x(0), y(0) {}
	explicit Point(NoInit) {}
	Point(PReal x, PReal y) : x(x), y(y) {}
	Point(const Point& p) : x(p.x), y(p.y) {}
	explicit Point(const PReal* d) : x(d[0]), y(d[1]) {}

	void set(PReal nx, PReal ny)
	{
		x = nx; y = ny;
	}

	void set(const Point& p)
	{
		x = p.x; y = p.y;
	}

	Point& operator=(const Point& p)
	{
		x = p.x; y = p.y;
		return *this;
	}

	PReal& operator[](int n)
	{
		assert(n >= 0 && n <= 1);
		return (&x)[n];
	}

	const PReal& operator[](int n) const
	{
		assert(n >= 0 && n <= 1);
		return (&x)[n];
	}

	PReal* ptr()
	{
		return &x;
	}

	const PReal* ptr() const
	{
		return &x;
	}

	Point operator+(const Point& p) const
	{
		return Point(x + p.x, y + p.y);
	}

	Point operator-(const Point& p) const
	{
		return Point(x - p.x, y - p.y);
	}

	Point& operator+=(const Point& p)
	{
		x += p.x;
		y += p.y;
		return *this;
	}

	Point& operator-=(const Point& p)
	{
		x -= p.x;
		y -= p.y;
		return *this;
	}

	Point operator*(PReal v) const
	{
		return Point(x * v, y * v);
	}

	Point operator/(PReal v) const
	{
		return Point(x / v, y / v);
	}

	Point& operator*=(PReal v)
	{
		x *= v;
		y *= v;
		return *this;
	}

	Point& operator/=(PReal v)
	{
		x /= v;
		y /= v;
		return *this;
	}

	// Negation
	Point operator-() const
	{
		return Point(-x, -y);
	}

	Point linearProduct(const Point& p) const
	{
		return Point(x * p.x, y * p.y);
	}

	Point linearDivision(const Point& p) const
	{
		return Point(x / p.x, y / p.y);
	}

	// Dot product
	PReal operator*(const Point& p) const
	{
		return x * p.x + y * p.y;
	}

	PReal dot(const Point& p) const
	{
		return x * p.x + y * p.y;
	}

	// Z component of the cross product of two vectors on the XY plane
	PReal cross(const Point& p) const
	{
		return x * p.y - y * p.x;
	}

	PReal norm2() const
	{
		return x * x + y * y;
	}

	PReal norm() const
	{
		return sqrt(norm2());
	}

	void normalizeWithNorm(PReal norm, PReal threshold=std::numeric_limits<PReal>::epsilon())
	{
		if(norm > threshold)
		{
			PReal invN = 1 / norm;
			x *= invN;
			y *= invN;
		}
	}

	Point normalizedWithNorm(PReal norm, PReal threshold=std::numeric_limits<PReal>::epsilon()) const
	{
		if(norm > threshold)
		{
			PReal invN = 1 / norm;
			return Point(x * invN, y * invN);
		}

		return Point();
	}

	void normalize(PReal threshold=std::numeric_limits<PReal>::epsilon())
	{
		normalizeWithNorm(norm(), threshold);
	}

	Point normalized(PReal threshold=std::numeric_limits<PReal>::epsilon()) const
	{
		return normalizedWithNorm(norm(), threshold);
	}

	// True if norm() is very close to 1
	bool isNormalized(PReal threshold=std::numeric_limits<PReal>::epsilon()*(PReal)10)
	{
		return abs(norm2() - static_cast<PReal>(1)) <= threshold;
	}

	bool operator==(const Point& p) const
	{
		return pFuzzyIsNull(x - p.x) && pFuzzyIsNull(y - p.y);
	}

	bool operator!=(const Point& p) const
	{
		return !pFuzzyIsNull(x - p.x) || !pFuzzyIsNull(y - p.y);
	}

	static Point max()
	{
		return Point(std::numeric_limits<PReal>::max(), std::numeric_limits<PReal>::max());
	}

	static Point zero() { return Point(0, 0); }
	static Point one() { return Point(1, 1); }

	static Point xAxis() { return Point(1, 0); }
	static Point yAxis() { return Point(0, 1); }
};

// Converts a coordinate from rectangular (Cartesian) coordinates to polar coordinates of the form (radius, theta)
Point toPolar(Point car)
{
	const PReal epsilon = (PReal)0.0000001;
	PReal theta;

	if(abs(car.x) < epsilon)
	{	// x == 0
		if(abs(car.y) < epsilon) theta = 0;
		else if(car.y > 0) theta = (PReal)M_PI / 2;
		else theta = ((PReal)M_PI * 3) / 2;
	}
	else if(car.x > 0)
	{
		if(car.y < 0) theta = atan(car.y / car.x) + 2 * (PReal)M_PI;
		else theta = atan(car.y / car.x);
	}
	else // car.x < 0
	{
		theta = (atan(car.y / car.x) + (PReal)M_PI);
	}

	return Point(car.norm(), theta);
}

// Converts a coordinate from polar coordinates of the form (radius, theta) to rectangular coordinates
Point fromPolar(Point pol)
{
	return Point(cos(pol.y) * pol.x, sin(pol.y) * pol.x);
}

} // namespace types

} // namespace panda

#undef EQUALITY_THRESHOLD

#endif // TYPES_POINT_H
