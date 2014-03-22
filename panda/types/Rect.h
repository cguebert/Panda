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

	Rect() : x1(0), y1(0), x2(0), y2(0) {}
	Rect(PReal x1, PReal y1, PReal x2, PReal y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
	Rect(const Rect& r) : x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2) {}
	Rect(const Point& p1, const Point& p2) : x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y) {}
	Rect(const Point& p, PReal w, PReal h) : x1(p.x), y1(p.y), x2(p.x + w), y2(p.y + h) {}

	void set(PReal nx1, PReal ny1, PReal nx2, PReal ny2)
	{
		x1 = nx1; y1 = ny1;
		x2 = nx2; y2 = ny2;
	}

	void canonicalize()
	{
		if(x1 > x2) std::swap(x1, x2);
		if(y1 > y2) std::swap(y1, y2);
	}
	Rect canonicalized() const
	{
		Rect r(*this);
		r.canonicalize();
		return r;
	}

	PReal width() const { return x2 - x1; }
	PReal height() const { return y2 - y1; }
	Point size() const { return Point(x2 - x1, y2 - y1); }
	PReal aspectRatio() const { return width() / height(); }
	PReal area() const { return width() * height(); }
	bool empty() const
	{
		PReal w = x2 - x1, h = y2 - y1;
		return w < 0 || h < 0 || pFuzzyIsNull(w) || pFuzzyIsNull(h);
	}

	void setWidth(PReal w) { x2 = x1 + w; }
	void setHeight(PReal h) { y2 = y1 + h; }
	void setSize(PReal w, PReal h) { x2 = x1 + w; y2 = y1 + h; }
	void setSize(const Point& s) { x2 = x1 + s.x; y2 = y1 + s.y; }

	PReal left() const { return x1; }
	PReal top() const { return y1; }
	PReal right() const { return x2; }
	PReal bottom() const { return y2; }

	void setLeft(PReal v) { x1 = v; }
	void setTop(PReal v) { y1 = v; }
	void setRight(PReal v) { x2 = v; }
	void setBottom(PReal v) { y2 = v; }

	Point topLeft() const { return Point(x1, y1); }
	Point bottomRight() const { return Point(x2, y2); }
	Point topRight() const { return Point(x2, y1); }
	Point bottomLeft() const { return Point(x1, y2); }
	Point center() const { return Point((x1+x2)/2, (y1+y2)/2); }

	void setTopLeft(const Point& p) { x1 = p.x; y1 = p.y; }
	void setBottomRight(const Point& p) { x2 = p.x; y2 = p.y; }
	void setTopRight(const Point& p) { x2 = p.x; y1 = p.y; }
	void setBottomLeft(const Point& p) { x1 = p.x; y2 = p.y; }

	void translate(const Point& p)
	{
		x1 += p.x; x2 += p.x;
		y1 += p.y; y2 += p.y;
	}
	void translate(PReal dx, PReal dy)
	{
		x1 += dx; x2 += dx;
		y1 += dy; y2 += dy;
	}
	Rect translated(const Point& p) const
	{
		return Rect(x1 + p.x, y1 + p.y, x2 + p.x, y2 + p.y);
	}
	Rect translated(PReal dx, PReal dy) const
	{
		return Rect(x1 + dx, y1 + dy, x2 + dx, y2 + dy);
	}

	void moveTo(const Point& p)
	{
		x2 += p.x - x1;
		y2 += p.y - y1;
		x1 = p.x;
		y1 = p.y;
	}
	void moveTo(PReal x, PReal y)
	{
		x2 += x - x1;
		y2 += y - y1;
		x1 = x;
		y1 = y;
	}

	void moveLeft(PReal v)
	{
		x2 += v - x1;
		x1 = v;
	}
	void moveTop(PReal v)
	{
		y2 += v - y1;
		y1 = v;
	}
	void moveRight(PReal v)
	{
		x1 += v - x2;
		x2 = v;
	}
	void moveBottom(PReal v)
	{
		y1 += v - y2;
		y2 = v;
	}

	void moveTopLeft(const Point& p)
	{
		moveLeft(p.x);
		moveTop(p.y);
	}
	void moveBottomRight(const Point& p)
	{
		moveRight(p.x);
		moveBottom(p.y);
	}
	void moveTopRight(const Point& p)
	{
		moveRight(p.x);
		moveTop(p.y);
	}
	void moveBottomLeft(const Point& p)
	{
		moveLeft(p.x);
		moveBottom(p.y);
	}

	void moveCenter(const Point& p)
	{
		PReal w = x2 - x1, h = y2 - y1;
		x1 = p.x - w / 2;
		y1 = p.y - h / 2;
		x2 = x1 + w;
		y2 = y1 + h;
	}

	void adjust(PReal dx1, PReal dy1, PReal dx2, PReal dy2)
	{
		x1 += dx1; y1 += dy1;
		x2 += dx2; y2 += dy2;
	}
	Rect adjusted(PReal dx1, PReal dy1, PReal dx2, PReal dy2) const
	{
		return Rect(x1 + dx1, y1 + dy1, x2 + dx2, y2 + dy2);
	}

	Rect operator|(const Rect &r) const;
	Rect operator&(const Rect &r) const;
	Rect& operator|=(const Rect &r)
	{
		*this = *this | r;
		return *this;
	}
	Rect& operator&=(const Rect &r)
	{
		*this = *this & r;
		return *this;
	}
	Rect intersected(const Rect& r) const
	{
		return *this & r;
	}
	Rect united(const Rect& r) const
	{
		return *this | r;
	}
	bool intersects(const Rect& r) const;

	bool contains(const Rect& r) const;
	bool contains(const Point& p) const
	{
		return p.x >= x1 && p.x <= x2
				&& p.y >= y1 && p.y <= y2;
	}
	bool contains(PReal x, PReal y) const
	{
		return contains(Point(x, y));
	}

	bool operator==(const Rect& r) const
	{
		return pFuzzyIsNull(x1 - r.x1) && pFuzzyIsNull(y1 - r.y1)
				&& pFuzzyIsNull(x2 - r.x2) && pFuzzyIsNull(y2 - r.y2);
	}

	bool operator!=(const Rect& r) const
	{
		return !pFuzzyIsNull(x1 - r.x1) || !pFuzzyIsNull(y1 - r.y1)
				|| !pFuzzyIsNull(x2 - r.x2) || !pFuzzyIsNull(y2 - r.y2);
	}
};

} // namespace types

} // namespace panda

#endif // TYPES_RECT_H
