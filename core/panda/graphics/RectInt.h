#ifndef GRAPHICS_RECTINT_H
#define GRAPHICS_RECTINT_H

#include <panda/graphics/PointInt.h>
#include <cmath>
#include <utility>

namespace panda
{

namespace graphics
{

class PANDA_CORE_API RectInt
{
public:
	int x1, y1, x2, y2;

	RectInt();
	RectInt(int x1, int y1, int x2, int y2);
	RectInt(const PointInt& p1, const PointInt& p2);
	RectInt(const PointInt& p, int w, int h);

	void set(int nx1, int ny1, int nx2, int ny2);

	void canonicalize();
	RectInt canonicalized() const;

	bool empty() const;

	int width() const;
	int height() const;
	PointInt size() const;
	int area() const;

	void setWidth(int w);
	void setHeight(int h);
	void setSize(int w, int h);
	void setSize(const PointInt& s);

	int left() const;
	int top() const;
	int right() const;
	int bottom() const;

	void setLeft(int v);
	void setTop(int v);
	void setRight(int v);
	void setBottom(int v);

	PointInt topLeft() const;
	PointInt bottomRight() const;
	PointInt topRight() const;
	PointInt bottomLeft() const;
	PointInt center() const;

	void setTopLeft(const PointInt& p);
	void setBottomRight(const PointInt& p);
	void setTopRight(const PointInt& p);
	void setBottomLeft(const PointInt& p);

	void translate(const PointInt& p);
	void translate(int dx, int dy);
	RectInt translated(const PointInt& p) const;
	RectInt translated(int dx, int dy) const;

	void moveTo(const PointInt& p);
	void moveTo(int x, int y);

	void moveLeft(int v);
	void moveTop(int v);
	void moveRight(int v);
	void moveBottom(int v);

	void moveTopLeft(const PointInt& p);
	void moveBottomRight(const PointInt& p);
	void moveTopRight(const PointInt& p);
	void moveBottomLeft(const PointInt& p);

	void moveCenter(const PointInt& p);

	void adjust(int dx1, int dy1, int dx2, int dy2);
	void adjust(const RectInt& r);

	RectInt adjusted(int dx1, int dy1, int dx2, int dy2) const;
	RectInt adjusted(const RectInt& r) const;

	bool contains(const PointInt& p) const;
	bool contains(int x, int y) const;
};

inline RectInt::RectInt()
: x1(0), y1(0), x2(0), y2(0) {}

inline RectInt::RectInt(int x1, int y1, int x2, int y2)
: x1(x1), y1(y1), x2(x2), y2(y2) {}

inline RectInt::RectInt(const PointInt& p1, const PointInt& p2)
: x1(x1), y1(y1), x2(x2), y2(y2) {}

inline RectInt::RectInt(const PointInt& p, int w, int h)
: x1(p.x), y1(p.y), x2(p.x + w), y2(p.y + h) {}

inline void RectInt::set(int nx1, int ny1, int nx2, int ny2)
{ x1 = nx1; y1 = ny1; x2 = nx2; y2 = ny2; }

inline void RectInt::canonicalize()
{ if(x1 > x2) std::swap(x1, x2); if(y1 > y2) std::swap(y1, y2); }

inline RectInt RectInt::canonicalized() const
{ RectInt r(*this); r.canonicalize(); return r; }

inline bool RectInt::empty() const 
{ return !width() || !height(); }

inline int RectInt::width() const
{ return x2 - x1; }

inline int RectInt::height() const
{ return y2 - y1; }

inline PointInt RectInt::size() const
{ return PointInt(width(), height()); }

inline int RectInt::area() const
{ return width() * height(); }

inline void RectInt::setWidth(int w)
{ x2 = x1 + w; }

inline void RectInt::setHeight(int h)
{ y2 = y1 + h; }

inline void RectInt::setSize(int w, int h)
{ x2 = x1 + w; y2 = y1 + h; }

inline void RectInt::setSize(const PointInt& s)
{ x2 = x1 + s.x; y2 = y1 + s.y; }

inline int RectInt::left() const
{ return x1; }

inline int RectInt::top() const
{ return y1; }

inline int RectInt::right() const
{ return x2; }

inline int RectInt::bottom() const
{ return y2; }

inline void RectInt::setLeft(int v)
{ x1 = v; }

inline void RectInt::setTop(int v)
{ y1 = v; }

inline void RectInt::setRight(int v)
{ x2 = v; }

inline void RectInt::setBottom(int v)
{ y2 = v; }

inline PointInt RectInt::topLeft() const
{ return PointInt(x1, y1); }

inline PointInt RectInt::bottomRight() const
{ return PointInt(x2, y2); }

inline PointInt RectInt::topRight() const
{ return PointInt(x2, y1); }

inline PointInt RectInt::bottomLeft() const
{ return PointInt(x1, y2); }

inline PointInt RectInt::center() const
{ return PointInt((x1+x2)/2, (y1+y2)/2); }

inline void RectInt::setTopLeft(const PointInt& p)
{ x1 = p.x; y1 = p.y; }

inline void RectInt::setBottomRight(const PointInt& p)
{ x2 = p.x; y2 = p.y; }

inline void RectInt::setTopRight(const PointInt& p)
{ x2 = p.x; y1 = p.y; }

inline void RectInt::setBottomLeft(const PointInt& p)
{ x1 = p.x; y2 = p.y; }


inline void RectInt::translate(const PointInt& p)
{ x1 += p.x; x2 += p.x; y1 += p.y; y2 += p.y; }

inline void RectInt::translate(int dx, int dy)
{ x1 += dx; x2 += dx; y1 += dy; y2 += dy; }

inline RectInt RectInt::translated(const PointInt& p) const
{ return RectInt(x1 + p.x, y1 + p.y, x2 + p.x, y2 + p.y); }

inline RectInt RectInt::translated(int dx, int dy) const
{ return RectInt(x1 + dx, y1 + dy, x2 + dx, y2 + dy); }

inline void RectInt::moveTo(const PointInt& p)
{ x2 += p.x - x1; y2 += p.y - y1; x1 = p.x; y1 = p.y; }

inline void RectInt::moveTo(int x, int y)
{ x2 += x - x1; y2 += y - y1; x1 = x; y1 = y; }

inline void RectInt::moveLeft(int v)
{ x2 += v - x1; x1 = v; }

inline void RectInt::moveTop(int v)
{ y2 += v - y1; y1 = v; }

inline void RectInt::moveRight(int v)
{ x1 += v - x2; x2 = v; }

inline void RectInt::moveBottom(int v)
{ y1 += v - y2; y2 = v; }

inline void RectInt::moveTopLeft(const PointInt& p)
{ moveLeft(p.x); moveTop(p.y); }

inline void RectInt::moveBottomRight(const PointInt& p)
{ moveRight(p.x); moveBottom(p.y); }

inline void RectInt::moveTopRight(const PointInt& p)
{ moveRight(p.x); moveTop(p.y); }

inline void RectInt::moveBottomLeft(const PointInt& p)
{ moveLeft(p.x); moveBottom(p.y); }

inline void RectInt::moveCenter(const PointInt& p)
{ int w = x2 - x1, h = y2 - y1; x1 = p.x - w / 2; y1 = p.y - h / 2; x2 = x1 + w; y2 = y1 + h; }

inline void RectInt::adjust(int dx1, int dy1, int dx2, int dy2)
{ x1 += dx1; y1 += dy1; x2 += dx2; y2 += dy2; }

inline void RectInt::adjust(const RectInt& r)
{ x1 += r.x1; y1 += r.y1; x2 += r.x2; y2 += r.y2; }

inline RectInt RectInt::adjusted(int dx1, int dy1, int dx2, int dy2) const
{ return RectInt(x1 + dx1, y1 + dy1, x2 + dx2, y2 + dy2); }

inline RectInt RectInt::adjusted(const RectInt& r) const
{ return RectInt(x1 + r.x1, y1 + r.y1, x2 + r.x2, y2 + r.y2); }

inline bool RectInt::contains(const PointInt& p) const
{ return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2; }

inline bool RectInt::contains(int x, int y) const
{ return contains(PointInt(x, y)); }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_RECTINT_H
