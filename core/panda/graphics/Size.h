#ifndef GRAPHICS_SIZE_H
#define GRAPHICS_SIZE_H

#include <panda/core.h>
#include <cmath>

namespace panda
{

namespace graphics
{

class PANDA_CORE_API Size
{
public:
	Size();
	Size(int width, int height);

	bool empty() const; // Returns true if the area is null or negative

	int width() const;
	int height() const;
	int& rwidth();
	int& rheight();
	void setWidth(int w);
	void setHeight(int h);

	void transpose();
	Size transposed() const;

	void scaleInside(int w, int h);	// The current size is scaled as large as possible inside the given rectangle
	void scaleInside(const Size& s);
	Size scaledInside(int w, int h) const;
	Size scaledInside(const Size& s) const;
	
	void scaleOutside(int w, int h); // The current size is scaled as small as possible outside the given rectangle
	void scaleOutside(const Size& s);
	Size scaledOutside(int w, int h) const;
	Size scaledOutside(const Size& s) const;

	Size& operator-=(const Size&);
	Size& operator*=(float c);
	Size& operator+=(const Size&);
	Size& operator/=(float c);

	friend bool operator==(const Size&, const Size&);
	friend bool operator!=(const Size&, const Size&);
	friend const Size operator+(const Size&, const Size&);
	friend const Size operator-(const Size&, const Size&);
	friend const Size operator*(const Size&, float);
	friend const Size operator*(float, const Size&);
	friend inline const Size operator/(const Size&, float);

protected:
	int m_width = -1, m_height = -1;
};

inline Size::Size()
{}

inline Size::Size(int w, int h) 
: m_width(w), m_height(h) {}

inline bool Size::empty() const
{ return m_width < 1 || m_height < 1; }

inline int Size::width() const
{ return m_width; }

inline int Size::height() const
{ return m_height; }

inline int& Size::rwidth()
{ return m_width; }

inline int& Size::rheight()
{ return m_height; }

inline void Size::setWidth(int w)
{ m_width = w; }

inline void Size::setHeight(int h)
{ m_height = h; }

inline void Size::transpose()
{ int tmp = m_width; m_width = m_height; m_height = tmp; }

inline Size Size::transposed() const
{ return Size(m_height, m_width); }

inline void Size::scaleInside(int w, int h)
{ scaleInside(Size(w, h)); }

inline void Size::scaleInside(const Size& s)
{ *this = scaledInside(s); }

inline Size Size::scaledInside(int w, int h) const
{ return scaledInside(Size(w, h)); }

inline void Size::scaleOutside(int w, int h)
{ scaleOutside(Size(w, h)); }

inline void Size::scaleOutside(const Size& s)
{ *this = scaledOutside(s); }

inline Size Size::scaledOutside(int w, int h) const
{ return scaledOutside(Size(w, h)); }

inline Size& Size::operator+=(const Size& s)
{ m_width += s.m_width; m_height += s.m_height; return *this; }

inline Size& Size::operator-=(const Size& s)
{ m_width -= s.m_width; m_height -= s.m_height; return *this; }

inline Size& Size::operator*=(float c)
{ m_width = std::lround(m_width * c); m_height = std::lround(m_height * c); return *this; }

inline bool operator==(const Size& s1, const Size& s2)
{ return s1.m_width == s2.m_width && s1.m_height == s2.m_height; }

inline bool operator!=(const Size& s1, const Size& s2)
{ return s1.m_width != s2.m_width || s1.m_height != s2.m_height; }

inline const Size operator+(const Size& s1, const Size& s2)
{ return Size(s1.m_width + s2.m_width, s1.m_height + s2.m_height); }

inline const Size operator-(const Size& s1, const Size& s2)
{ return Size(s1.m_width - s2.m_width, s1.m_height - s2.m_height); }

inline const Size operator*(const Size& s, float c)
{ return Size(std::lround(s.m_width * c), std::lround(s.m_height * c)); }

inline const Size operator*(float c, const Size& s)
{ return Size(std::lround(s.m_width * c), std::lround(s.m_height * c)); }

inline Size& Size::operator/=(float c)
{ m_width = std::lround(m_width / c); m_height = std::lround(m_height / c); return *this; }

inline const Size operator/(const Size& s, float c)
{ return Size(std::lround(s.m_width / c), std::lround(s.m_height / c)); }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_SIZE_H
