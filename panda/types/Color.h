#ifndef TYPES_COLOR_H
#define TYPES_COLOR_H

#include <panda/helper/system/Config.h>

#include <cstdint>
#include <cassert>

namespace panda
{

namespace types
{

class Color
{
public:
	float a, r, g, b;

	Color();
	Color(float r, float g, float b, float a = 1.0f);
	Color(const Color& c);

	void set(float a, float r, float g, float b);
	void set(const Color& c);

	Color& operator=(const Color& c);

	Color operator+(const Color& c) const;
	Color operator-(const Color& c) const;
	Color operator*(const Color& c) const;
	Color operator/(const Color& c) const;

	Color operator+(float v) const;
	Color operator-(float v) const;
	Color operator*(float v) const;
	Color operator/(float v) const;

	Color& operator+=(const Color& c);
	Color& operator-=(const Color& c);
	Color& operator*=(const Color& c);
	Color& operator/=(const Color& c);

	Color& operator+=(float v);
	Color& operator-=(float v);
	Color& operator*=(float v);
	Color& operator/=(float v);

	bool operator==(const Color& c) const;
	bool operator!=(const Color& c) const;

	float& operator[](int n);
	const float& operator[](int n) const;

	float* ptr();
	const float* ptr() const;

	operator float*();
	operator const float*() const;

	Color premultiplied() const;

	void getHsv(float& hue, float& saturation, float& value, float& alpha) const;
	static Color fromHsv(float hue, float saturation, float value, float alpha = 1.0f);
	static Color fromHex(uint32_t hexValue);
};

template <typename T> inline Color operator*(T f, const Color& c)
{ return Color(c.r * f, c.g * f, c.b * f, c.a * f); }

inline Color::Color()
: r(0), g(0), b(0), a(0) { }

inline Color::Color(float r, float g, float b, float a)
: r(r), g(g), b(b), a(a) { }

inline Color::Color(const Color& c)
: r(c.r), g(c.g), b(c.b), a(c.a) { }

inline void Color::set(float aA, float aR, float aG, float aB)
{ r = aR; g = aG; b = aB; a = aA; }

inline void Color::set(const Color &c)
{ r = c.r; g = c.g; b = c.b; a = c.a; }

inline Color& Color::operator=(const Color& c)
{ set(c); return *this; }

inline Color Color::operator+(const Color& c) const
{ return Color(r + c.r, g + c.g, b + c.b, a + c.a); }

inline Color Color::operator-(const Color& c) const
{ return Color(r - c.r, g - c.g, b - c.b, a - c.a); }

inline Color Color::operator*(const Color& c) const
{ return Color(r * c.r, g * c.g, b * c.b, a * c.a); }

inline Color Color::operator/(const Color& c) const
{ return Color(r / c.r, g / c.g, b / c.b, a / c.a); }

inline Color Color::operator+(float v) const
{ return Color(r + v, g + v, b + v, a + v); }

inline Color Color::operator-(float v) const
{ return Color(r - v, g - v, b - v, a - v); }

inline Color Color::operator*(float v) const
{ return Color(r * v, g * v, b * v, a * v); }

inline Color Color::operator/(float v) const
{ return Color(r / v, g / v, b / v, a / v); }

inline Color& Color::operator+=(const Color& c)
{ r += c.r; g += c.g; b += c.b; a += c.a; return *this; }

inline Color& Color::operator-=(const Color& c)
{ r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }

inline Color& Color::operator*=(const Color& c)
{ r *= c.r; g *= c.g; b *= c.b; a *= c.a; return *this; }

inline Color& Color::operator/=(const Color& c)
{ r /= c.r; g /= c.g; b /= c.b; a /= c.a; return *this; }

inline Color& Color::operator+=(float v)
{ r += v; g += v; b += v; a += v; return *this; }

inline Color& Color::operator-=(float v)
{ r -= v; g -= v; b -= v; a -= v; return * this; }

inline Color& Color::operator*=(float v)
{ r *= v; g *= v; b *= v; a *= v; return * this; }

inline Color& Color::operator/=(float v)
{ r /= v; g /= v; b /= v; a /= v; return * this; }

inline bool Color::operator==(const Color& c) const
{ return (r == c.r) && (g == c.g) && (b == c.b) && (a == c.a ); }

inline bool Color::operator!=(const Color& c) const
{ return !(*this == c); }

inline float& Color::operator[](int n)
{
	assert(n >= 0 && n <= 3);
	return (&a)[n];
}

inline const float& Color::operator[](int n) const
{
	assert(n >= 0 && n <= 3);
	return (&a)[n];
}

inline float* Color::ptr()
{ return &a; }

inline const float* Color::ptr() const
{ return &a; }

inline Color::operator float*()
{ return ptr(); }

inline Color::operator const float*() const
{ return ptr(); }

inline Color Color::premultiplied() const
{ return Color(r * a, g * a, b * a, a); }

} // namespace types

} // namespace panda

#endif // TYPES_COLOR_H
