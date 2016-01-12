#ifndef TYPES_COLOR_H
#define TYPES_COLOR_H

#include <panda/core.h>

#ifndef PANDA_BUILD_CORE
#include <panda/types/Animation.h>
#include <panda/Data.h>
#endif

#include <cstdint>
#include <cassert>

namespace panda
{

namespace types
{

class PANDA_CORE_API Color
{
public:
	float r, g, b, a; // Format of OpenGL

	Color();
	Color(float r, float g, float b, float a = 1.0f);
	Color(const Color& c);

	void set(float r, float g, float b, float a);
	void set(const Color& c);

	void get(float& r, float& g, float& b, float& a) const;

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

	float* data();
	const float* data() const;

	Color premultiplied() const;
	Color bounded() const;	/// make sure all components are [0:1]

	void getHsv(float& hue, float& saturation, float& value, float& alpha) const;
	static Color fromHsv(float hue, float saturation, float value, float alpha = 1.0f);

	uint32_t toHex() const;
	static Color fromHex(uint32_t hexValue);

	static Color fromByte(const unsigned char* px);

	static Color null();
	static Color black();
	static Color white();
};

template <typename T> inline Color operator*(T f, const Color& c)
{ return Color(c.r * f, c.g * f, c.b * f, c.a * f); }

inline Color::Color()
: r(0), g(0), b(0), a(1) { }

inline Color::Color(float r, float g, float b, float a)
: r(r), g(g), b(b), a(a) { }

inline Color::Color(const Color& c)
: r(c.r), g(c.g), b(c.b), a(c.a) { }

inline void Color::set(float aR, float aG, float aB, float aA)
{ r = aR; g = aG; b = aB; a = aA; }

inline void Color::set(const Color &c)
{ r = c.r; g = c.g; b = c.b; a = c.a; }

inline void Color::get(float &aR, float &aG, float &aB, float &aA) const
{ aR = r; aG = g; aB = b; aA = a; }

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
	return (&r)[n];
}

inline const float& Color::operator[](int n) const
{
	assert(n >= 0 && n <= 3);
	return (&r)[n];
}

inline float* Color::data()
{ return &r; }

inline const float* Color::data() const
{ return &r; }

inline Color Color::premultiplied() const
{ return Color(r * a, g * a, b * a, a); }

inline Color Color::null()
{ return Color(0, 0, 0, 0); }

inline Color Color::black()
{ return Color(0, 0, 0, 1); }

inline Color Color::white()
{ return Color(1, 1, 1, 1); }

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<Color>;
extern template class PANDA_CORE_API Data< std::vector<Color> >;
extern template class PANDA_CORE_API Animation<Color>;
extern template class PANDA_CORE_API Data< Animation<Color> >;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_COLOR_H
