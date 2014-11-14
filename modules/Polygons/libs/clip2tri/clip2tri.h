/*
 * clip2tri.h
 *
 * Authors: kaen, raptor, sam686, watusimoto
 *
 * Originally from the bitfighter source code
 */

#ifndef CLIP2TRI_H_
#define CLIP2TRI_H_

#include <vector>

#include "../clipper/clipper.hpp"

using namespace std;
using namespace ClipperLib;

namespace c2t
{

typedef signed int       S32;
typedef signed long long S64;
typedef unsigned int     U32;
typedef float            F32;
typedef double           F64;


struct Point
{
   F32 x;
   F32 y;

   Point();
   Point(const Point &pt);

   template<class T, class U>
   Point(T in_x, U in_y) { x = static_cast<F32>(in_x); y = static_cast<F32>(in_y); }

   friend inline bool operator== (const Point& a, const Point& b)
   {
	 return a.x == b.x && a.y == b.y;
   }
   friend inline bool operator!= (const Point& a, const Point& b)
   {
	 return !(a==b);
   }
};

void triangulate(const vector<vector<Point> > inputPolygons, const vector<Point> boundingPolygon, vector<Point> &outputTriangles);

} /* namespace c2t */

#endif /* CLIP2TRI_H_ */
