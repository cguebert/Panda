#ifndef GRAPHICS_POINTINT_H
#define GRAPHICS_POINTINT_H

#include <panda/core.h>
#include <cmath>

namespace panda
{

namespace graphics
{

class PANDA_CORE_API PointInt
{
public:
	int x, y;

	PointInt();
	PointInt(int xos, int yos);

	PointInt& operator+=(const PointInt& p);
	PointInt& operator-=(const PointInt& p);

	PointInt& operator*=(float v);
	PointInt& operator*=(int v);

	PointInt& operator/=(float v);

	friend bool operator==(const PointInt&, const PointInt&);
	friend bool operator!=(const PointInt&, const PointInt&);
	friend const PointInt operator+(const PointInt&, const PointInt&);
	friend const PointInt operator-(const PointInt&, const PointInt&);
	friend const PointInt operator*(const PointInt&, float);
	friend const PointInt operator*(float, const PointInt&);
	friend const PointInt operator*(const PointInt&, int);
	friend const PointInt operator*(int, const PointInt&);
	friend const PointInt operator+(const PointInt&);
	friend const PointInt operator-(const PointInt&);
	friend const PointInt operator/(const PointInt&, float);
};

inline int dotProduct(const PointInt& p1, const PointInt& p2)
{ return p1.x * p2.x + p1.y * p2.y; }

inline PointInt::PointInt()
: x(0), y(0) {}

inline PointInt::PointInt(int x, int y) 
: x(x), y(y) {}

inline PointInt& PointInt::operator+=(const PointInt& p)
{ x += p.x; y += p.y; return *this; }

inline PointInt& PointInt::operator-=(const PointInt& p)
{ x -= p.x; y -= p.y; return *this; }

inline PointInt& PointInt::operator*=(float v)
{ x = std::lround(x * v); y = std::lround(y * v); return *this; }

inline PointInt& PointInt::operator*=(int v)
{ x = x * v; y = y * v; return *this; }

inline bool operator==(const PointInt& p1, const PointInt& p2)
{ return p1.x == p2.x && p1.y == p2.y; }

inline bool operator!=(const PointInt& p1, const PointInt& p2)
{ return p1.x != p2.x || p1.y != p2.y; }

inline const PointInt operator+(const PointInt& p1, const PointInt& p2)
{ return PointInt(p1.x + p2.x, p1.y + p2.y); }

inline const PointInt operator-(const PointInt& p1, const PointInt& p2)
{ return PointInt(p1.x - p2.x, p1.y - p2.y); }

inline const PointInt operator*(const PointInt& p, float v)
{ return PointInt(std::lround(p.x * v), std::lround(p.y * v)); }

inline const PointInt operator*(const PointInt& p, int v)
{ return PointInt(p.x * v, p.y * v); }

inline const PointInt operator*(float v, const PointInt& p)
{ return PointInt(std::lround(p.x * v), std::lround(p.y * v)); }

inline const PointInt operator*(int v, const PointInt& p)
{ return PointInt(p.x * v, p.y * v); }

inline const PointInt operator+(const PointInt& p)
{ return p; }

inline const PointInt operator-(const PointInt& p)
{ return PointInt(-p.x, -p.y); }

inline PointInt& PointInt::operator/=(float v)
{ x = std::lround(x / v); y = std::lround(y / v); return *this; }

inline const PointInt operator/(const PointInt& p, float v)
{ return PointInt(std::lround(p.x / v), std::lround(p.y / v)); }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_POINTINT_H
