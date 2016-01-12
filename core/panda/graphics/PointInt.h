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
	PointInt();
	PointInt(int xos, int yos);

	int x() const;
	int y() const;
	int& rx();
	int& ry();
	void setX(int);
	void setY(int);

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

protected:
	int m_x = 0, m_y = 0;
};

inline int dotProduct(const PointInt& p1, const PointInt& p2)
{ return p1.x() * p2.x() + p1.y() * p2.y(); }

inline PointInt::PointInt() {}

inline PointInt::PointInt(int x, int y) 
: m_x(x), m_y(x) {}

inline int PointInt::x() const
{ return m_x; }

inline int PointInt::y() const
{ return m_y; }

inline void PointInt::setX(int x)
{ m_x = x; }

inline void PointInt::setY(int y)
{ m_y = y; }

inline int &PointInt::rx()
{ return m_x; }

inline int &PointInt::ry()
{ return m_y; }

inline PointInt& PointInt::operator+=(const PointInt& p)
{ m_x += p.m_x; m_y += p.m_y; return *this; }

inline PointInt& PointInt::operator-=(const PointInt& p)
{ m_x -= p.m_x; m_y -= p.m_y; return *this; }

inline PointInt& PointInt::operator*=(float v)
{ m_x = std::lround(m_x * v); m_y = std::lround(m_y * v); return *this; }

inline PointInt& PointInt::operator*=(int v)
{ m_x = m_x * v; m_y = m_y * v; return *this; }

inline bool operator==(const PointInt& p1, const PointInt& p2)
{ return p1.m_x == p2.m_x && p1.m_y == p2.m_y; }

inline bool operator!=(const PointInt& p1, const PointInt& p2)
{ return p1.m_x != p2.m_x || p1.m_y != p2.m_y; }

inline const PointInt operator+(const PointInt& p1, const PointInt& p2)
{ return PointInt(p1.m_x + p2.m_x, p1.m_y + p2.m_y); }

inline const PointInt operator-(const PointInt& p1, const PointInt& p2)
{ return PointInt(p1.m_x - p2.m_x, p1.m_y - p2.m_y); }

inline const PointInt operator*(const PointInt& p, float v)
{ return PointInt(std::lround(p.m_x * v), std::lround(p.m_y * v)); }

inline const PointInt operator*(const PointInt& p, int v)
{ return PointInt(p.m_x * v, p.m_y * v); }

inline const PointInt operator*(float v, const PointInt& p)
{ return PointInt(std::lround(p.m_x * v), std::lround(p.m_y * v)); }

inline const PointInt operator*(int v, const PointInt& p)
{ return PointInt(p.m_x * v, p.m_y * v); }

inline const PointInt operator+(const PointInt& p)
{ return p; }

inline const PointInt operator-(const PointInt& p)
{ return PointInt(-p.m_x, -p.m_y); }

inline PointInt& PointInt::operator/=(float v)
{ m_x = std::lround(m_x / v); m_y = std::lround(m_y / v); return *this; }

inline const PointInt operator/(const PointInt& p, float v)
{ return PointInt(std::lround(p.m_x / v), std::lround(p.m_y / v)); }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_POINTINT_H
