#ifndef TYPES_RECT_H
#define TYPES_RECT_H

#include <panda/types/Point.h>
#include <utility>

namespace panda
{

namespace types
{

class Rect
{
public:
	PReal x1, y1, x2, y2;

	Rect();
	Rect(PReal x1, PReal y1, PReal x2, PReal y2);
	Rect(const Rect& r);
	Rect(const Point& p1, const Point& p2);
	Rect(const Point& p, PReal w, PReal h);

	void set(PReal nx1, PReal ny1, PReal nx2, PReal ny2);

	void canonicalize();
	Rect canonicalized() const;

	PReal width() const;
	PReal height() const;
	Point size() const;
	PReal aspectRatio() const;
	PReal area() const;
	bool empty() const;

	void setWidth(PReal w);
	void setHeight(PReal h);
	void setSize(PReal w, PReal h);
	void setSize(const Point& s);

	PReal left() const;
	PReal top() const;
	PReal right() const;
	PReal bottom() const;

	void setLeft(PReal v);
	void setTop(PReal v);
	void setRight(PReal v);
	void setBottom(PReal v);

	Point topLeft() const;
	Point bottomRight() const;
	Point topRight() const;
	Point bottomLeft() const;
	Point center() const;

	void setTopLeft(const Point& p);
	void setBottomRight(const Point& p);
	void setTopRight(const Point& p);
	void setBottomLeft(const Point& p);

	void translate(const Point& p);
	void translate(PReal dx, PReal dy);
	Rect translated(const Point& p) const;
	Rect translated(PReal dx, PReal dy) const;

	void moveTo(const Point& p);
	void moveTo(PReal x, PReal y);

	void moveLeft(PReal v);
	void moveTop(PReal v);
	void moveRight(PReal v);
	void moveBottom(PReal v);

	void moveTopLeft(const Point& p);
	void moveBottomRight(const Point& p);
	void moveTopRight(const Point& p);
	void moveBottomLeft(const Point& p);

	void moveCenter(const Point& p);

	void adjust(PReal dx1, PReal dy1, PReal dx2, PReal dy2);
	void adjust(const Rect& r);

	Rect adjusted(PReal dx1, PReal dy1, PReal dx2, PReal dy2) const;
	Rect adjusted(const Rect& r) const;

	Rect operator|(const Rect &r) const; /// Union
	Rect& operator|=(const Rect &r);
	Rect united(const Rect& r) const;

	Rect operator&(const Rect &r) const; /// Intersection
	Rect& operator&=(const Rect &r);
	Rect intersected(const Rect& r) const;

	bool intersects(const Rect& r) const;

	bool contains(const Rect& r) const;
	bool contains(const Point& p) const;
	bool contains(PReal x, PReal y) const;

	bool operator==(const Rect& r) const;
	bool operator!=(const Rect& r) const;
};

inline Rect::Rect()
: x1(0), y1(0), x2(0), y2(0) { }

inline Rect::Rect(PReal x1, PReal y1, PReal x2, PReal y2)
: x1(x1), y1(y1), x2(x2), y2(y2) { }

inline Rect::Rect(const Rect& r)
: x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2) { }

inline Rect::Rect(const Point& p1, const Point& p2)
: x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y) { }

inline Rect::Rect(const Point& p, PReal w, PReal h)
: x1(p.x), y1(p.y), x2(p.x + w), y2(p.y + h) { }

inline void Rect::set(PReal nx1, PReal ny1, PReal nx2, PReal ny2)
{ x1 = nx1; y1 = ny1; x2 = nx2; y2 = ny2; }

inline void Rect::canonicalize()
{ if(x1 > x2) std::swap(x1, x2); if(y1 > y2) std::swap(y1, y2); }

inline Rect Rect::canonicalized() const
{ Rect r(*this); r.canonicalize(); return r; }

inline PReal Rect::width() const
{ return x2 - x1; }

inline PReal Rect::height() const
{ return y2 - y1; }

inline Point Rect::size() const
{ return Point(x2 - x1, y2 - y1); }

inline PReal Rect::aspectRatio() const
{ return width() / height(); }

inline PReal Rect::area() const
{ return width() * height(); }

inline bool Rect::empty() const
{ PReal w = x2 - x1, h = y2 - y1; return w < 0 || h < 0 || pFuzzyIsNull(w) || pFuzzyIsNull(h); }

inline void Rect::setWidth(PReal w)
{ x2 = x1 + w; }

inline void Rect::setHeight(PReal h)
{ y2 = y1 + h; }

inline void Rect::setSize(PReal w, PReal h)
{ x2 = x1 + w; y2 = y1 + h; }

inline void Rect::setSize(const Point& s)
{ x2 = x1 + s.x; y2 = y1 + s.y; }

inline PReal Rect::left() const
{ return x1; }

inline PReal Rect::top() const
{ return y1; }

inline PReal Rect::right() const
{ return x2; }

inline PReal Rect::bottom() const
{ return y2; }

inline void Rect::setLeft(PReal v)
{ x1 = v; }

inline void Rect::setTop(PReal v)
{ y1 = v; }

inline void Rect::setRight(PReal v)
{ x2 = v; }

inline void Rect::setBottom(PReal v)
{ y2 = v; }

inline Point Rect::topLeft() const
{ return Point(x1, y1); }

inline Point Rect::bottomRight() const
{ return Point(x2, y2); }

inline Point Rect::topRight() const
{ return Point(x2, y1); }

inline Point Rect::bottomLeft() const
{ return Point(x1, y2); }

inline Point Rect::center() const
{ return Point((x1+x2)/2, (y1+y2)/2); }

inline void Rect::setTopLeft(const Point& p)
{ x1 = p.x; y1 = p.y; }

inline void Rect::setBottomRight(const Point& p)
{ x2 = p.x; y2 = p.y; }

inline void Rect::setTopRight(const Point& p)
{ x2 = p.x; y1 = p.y; }

inline void Rect::setBottomLeft(const Point& p)
{ x1 = p.x; y2 = p.y; }

inline void Rect::translate(const Point& p)
{ x1 += p.x; x2 += p.x; y1 += p.y; y2 += p.y; }

inline void Rect::translate(PReal dx, PReal dy)
{ x1 += dx; x2 += dx; y1 += dy; y2 += dy; }

inline Rect Rect::translated(const Point& p) const
{ return Rect(x1 + p.x, y1 + p.y, x2 + p.x, y2 + p.y); }

inline Rect Rect::translated(PReal dx, PReal dy) const
{ return Rect(x1 + dx, y1 + dy, x2 + dx, y2 + dy); }

inline void Rect::moveTo(const Point& p)
{ x2 += p.x - x1; y2 += p.y - y1; x1 = p.x; y1 = p.y; }

inline void Rect::moveTo(PReal x, PReal y)
{ x2 += x - x1; y2 += y - y1; x1 = x; y1 = y; }

inline void Rect::moveLeft(PReal v)
{ x2 += v - x1; x1 = v; }

inline void Rect::moveTop(PReal v)
{ y2 += v - y1; y1 = v; }

inline void Rect::moveRight(PReal v)
{ x1 += v - x2; x2 = v; }

inline void Rect::moveBottom(PReal v)
{ y1 += v - y2; y2 = v; }

inline void Rect::moveTopLeft(const Point& p)
{ moveLeft(p.x); moveTop(p.y); }

inline void Rect::moveBottomRight(const Point& p)
{ moveRight(p.x); moveBottom(p.y); }

inline void Rect::moveTopRight(const Point& p)
{ moveRight(p.x); moveTop(p.y); }

inline void Rect::moveBottomLeft(const Point& p)
{ moveLeft(p.x); moveBottom(p.y); }

inline void Rect::moveCenter(const Point& p)
{ PReal w = x2 - x1, h = y2 - y1; x1 = p.x - w / 2; y1 = p.y - h / 2; x2 = x1 + w; y2 = y1 + h; }

inline void Rect::adjust(PReal dx1, PReal dy1, PReal dx2, PReal dy2)
{ x1 += dx1; y1 += dy1; x2 += dx2; y2 += dy2; }

inline void Rect::adjust(const Rect& r)
{ x1 += r.x1; y1 += r.y1; x2 += r.x2; y2 += r.y2; }

inline Rect Rect::adjusted(PReal dx1, PReal dy1, PReal dx2, PReal dy2) const
{ return Rect(x1 + dx1, y1 + dy1, x2 + dx2, y2 + dy2); }

inline Rect Rect::adjusted(const Rect& r) const
{ return Rect(x1 + r.x1, y1 + r.y1, x2 + r.x2, y2 + r.y2); }

inline Rect& Rect::operator|=(const Rect &r)
{ *this = *this | r; return *this; }

inline Rect Rect::united(const Rect& r) const
{ return *this | r; }

inline Rect& Rect::operator&=(const Rect &r)
{ *this = *this & r; return *this; }

inline Rect Rect::intersected(const Rect& r) const
{ return *this & r; }

inline bool Rect::contains(const Point& p) const
{ return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2; }

inline bool Rect::contains(PReal x, PReal y) const
{ return contains(Point(x, y)); }

inline bool Rect::operator==(const Rect& r) const
{ return pFuzzyIsNull(x1 - r.x1) && pFuzzyIsNull(y1 - r.y1) && pFuzzyIsNull(x2 - r.x2) && pFuzzyIsNull(y2 - r.y2); }

inline bool Rect::operator!=(const Rect& r) const
{ return !pFuzzyIsNull(x1 - r.x1) || !pFuzzyIsNull(y1 - r.y1) || !pFuzzyIsNull(x2 - r.x2) || !pFuzzyIsNull(y2 - r.y2); }

} // namespace types

} // namespace panda

#endif // TYPES_RECT_H
