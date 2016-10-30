#include <panda/graphview/graphics/DrawPath.h>
#include <panda/graphview/graphics/poly2tri/poly2tri.h>

#include <algorithm>
#include <cassert>

using panda::types::Point;

namespace
{

	static void pathBezierToCasteljau(std::vector<Point>& path, Point p1, Point p2, Point p3, Point p4, float tess_tol, int level)
	{
		auto p14 = p4 - p1, p42 = p2 - p4, p43 = p3 - p4;
		float d2 = fabs(p42.cross(p14));
		float d3 = fabs(p43.cross(p14));
		if ((d2+d3) * (d2+d3) < tess_tol * p14.norm2()) 
		{
			path.push_back(p4);
		}
		else if (level < 10)
		{
			auto p12 = (p1 + p2)*0.5f, p23 = (p2 + p3)*0.5f, p34 = (p3 + p4)*0.5f;
			auto p123 = (p12 + p23)*0.5f, p234 = (p23 + p34)*0.5f;
			auto p1234 = (p123 + p234)*0.5f;

			pathBezierToCasteljau(path, p1, p12,p123, p1234, tess_tol, level+1); 
			pathBezierToCasteljau(path, p1234, p234, p34, p4, tess_tol, level+1); 
		}
	}

	using panda::types::Point;

	// Copyright 2000 softSurfer, 2012 Dan Sunday
	// This code may be freely used and modified for any purpose
	// providing that this copyright notice is included with it.
	// SoftSurfer makes no warranty for this code, and cannot be held
	// liable for any real or imagined damage resulting from its use.
	// Users of this code must verify correctness for their application.

	// http://geomalgorithms.com/a03-_inclusion.html

	// isLeft(): tests if a point is Left|On|Right of an infinite line.
	//    Input:  three points P0, P1, and P2
	//    Return: >0 for P2 left of the line through P0 and P1
	//            =0 for P2  on the line
	//            <0 for P2  right of the line
	//    See: Algorithm 1 "Area of Triangles and Polygons"
	inline float isLeft( Point P0, Point P1, Point P2 )
	{
		return ( (P1.x - P0.x) * (P2.y - P0.y)
				- (P2.x -  P0.x) * (P1.y - P0.y) );
	}

	// wn_PnPoly(): winding number test for a point in a polygon
	//      Input:   P = a point,
	//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
	//      Return:  wn = the winding number (=0 only when P is outside)
	int wn_PnPoly( Point P, const Point* V, int n )
	{
		int    wn = 0;    // the  winding number counter

		// loop through all edges of the polygon
		for (int i=0; i<n; i++) {   // edge from V[i] to  V[i+1]
			if (V[i].y <= P.y) {          // start y <= P.y
				if (V[i+1].y  > P.y)      // an upward crossing
					 if (isLeft( V[i], V[i+1], P) > 0)  // P left of  edge
						 ++wn;            // have  a valid up intersect
			}
			else {                        // start y > P.y (no test needed)
				if (V[i+1].y  <= P.y)     // a downward crossing
					 if (isLeft( V[i], V[i+1], P) < 0)  // P right of  edge
						 --wn;            // have  a valid down intersect
			}
		}
		return wn;
	}

} // Unnamed namespace

namespace panda
{

namespace graphview
{

namespace graphics
{

void DrawPath::arcToFast(const Point& centre, float radius, int amin, int amax)
{
	static const int circle_vtx_count = 12;
	static Point circle_vtx[circle_vtx_count];
	static bool circle_vtx_builds = false;
	if (!circle_vtx_builds)
	{
		static const float pi2 = 2 * (float)M_PI;
		for (int i = 0; i < circle_vtx_count; i++)
		{
			const float a = ((float)i / (float)circle_vtx_count) * pi2;
			circle_vtx[i].x = cosf(a);
			circle_vtx[i].y = sinf(a);
		}
		circle_vtx_builds = true;
	}

	if (amin > amax) return;
	if (radius == 0.0f)
	{
		m_points.push_back(centre);
	}
	else
	{
		m_points.reserve(m_points.size() + (amax - amin + 1));
		for (int a = amin; a <= amax; a++)
		{
			const Point& c = circle_vtx[a % circle_vtx_count];
			m_points.push_back(Point(centre.x + c.x * radius, centre.y + c.y * radius));
		}
	}
}

void DrawPath::arcTo(const Point& centre, float radius, float amin, float amax, int num_segments)
{
	if (radius == 0.0f)
		m_points.push_back(centre);
	m_points.reserve(m_points.size() + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = amin + ((float)i / (float)num_segments) * (amax - amin);
		m_points.push_back(Point(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
	}
}

void DrawPath::arcTo(const pRect& rect, float start, float arc_angle, float precision)
{
	float radW = fabs(rect.width() / 2.0f), radH = fabs(rect.height() / 2.0f);
	float maxRadius = std::max(radW, radH);
	int num_segments = static_cast<int>(fabs(arc_angle) / acosf(1.f - precision / maxRadius));
	num_segments = std::max(3, num_segments);

	const auto& center = rect.center();
	m_points.reserve(m_points.size() + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = start + ((float)i / (float)num_segments) * arc_angle;
		m_points.push_back(Point(center.x + cosf(a) * radW, center.y + sinf(a) * radH));
	}
}

void DrawPath::arcToDegrees(const pRect& rect, float start, float arc_angle, float precision)
{
	const float pi180 = static_cast<float>(M_PI) / 180.0f;
	arcTo(rect, start * pi180, arc_angle * pi180, precision);
}

void DrawPath::bezierCurveTo(const Point& p2, const Point& p3, const Point& p4, int num_segments)
{
	Point p1 = m_points.back();
	if (num_segments == 0)
	{
		// Auto-tessellated
		const float curveTessellationTol = 1.25f;
		pathBezierToCasteljau(m_points, p1, p2, p3, p4, curveTessellationTol, 0);
	}
	else
	{
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
		{
			float t = t_step * i_step;
			float u = 1.0f - t;
			m_points.push_back(u*u*u*p1 + 3*u*u*t*p2 + 3*u*t*t*p3 + t*t*t*p4);
		}
	}
}

void DrawPath::rect(const pRect& rect, float rounding, int rounding_corners)
{
	float r = rounding;
	r = std::min(r, fabsf(rect.width()) * ( ((rounding_corners&(1|2))==(1|2)) || ((rounding_corners&(4|8))==(4|8)) ? 0.5f : 1.0f ) - 1.0f);
	r = std::min(r, fabsf(rect.height()) * ( ((rounding_corners&(1|8))==(1|8)) || ((rounding_corners&(2|4))==(2|4)) ? 0.5f : 1.0f ) - 1.0f);

	Point a = rect.topLeft(), b = rect.bottomRight();
	if (r <= 0.0f || rounding_corners == 0)
	{
		lineTo(a);
		lineTo(Point(b.x,a.y));
		lineTo(b);
		lineTo(Point(a.x,b.y));
	}
	else
	{
		const float r0 = (rounding_corners & 1) ? r : 0.0f;
		const float r1 = (rounding_corners & 2) ? r : 0.0f;
		const float r2 = (rounding_corners & 4) ? r : 0.0f;
		const float r3 = (rounding_corners & 8) ? r : 0.0f;
		arcToFast(Point(a.x+r0,a.y+r0), r0, 6, 9);
		arcToFast(Point(b.x-r1,a.y+r1), r1, 9, 12);
		arcToFast(Point(b.x-r2,b.y-r2), r2, 0, 3);
		arcToFast(Point(a.x+r3,b.y-r3), r3, 3, 6);
	}
}

void DrawPath::removeDuplicates()
{
	auto last = std::unique(m_points.begin(), m_points.end(), [](const Point& lhs, const Point& rhs) {
		return (rhs - lhs).norm2() < 0.001f;
	});
	m_points.erase(last, m_points.end());
}

bool DrawPath::contains(const Point& pos)
{
	return wn_PnPoly(pos, m_points.data(), m_points.size() - 1) != 0;
}

DrawMesh DrawPath::triangulate() const
{
	if (m_points.size() < 3)
		return DrawMesh();

	auto inputPoints = m_points;
	auto last = std::unique(inputPoints.begin(), inputPoints.end(), [](const Point& lhs, const Point& rhs) {
		return (rhs - lhs).norm2() < 0.001f;
	});
	inputPoints.erase(last, inputPoints.end());

	std::vector<p2t::Point> p2tPoints;
	for (const auto& pt : inputPoints)
		p2tPoints.emplace_back(pt.x, pt.y);
	if (p2tPoints.front() == p2tPoints.back())
		p2tPoints.pop_back();
	
	// Poly2Tri uses pointers to points
	std::vector<p2t::Point*> contour;
	contour.reserve(p2tPoints.size());
	for (auto& pt : p2tPoints)
		contour.push_back(&pt);
	
	// Set the contour in poly2tri
	p2t::CDT cdt(contour);

	// Do the actual triangulation
	cdt.Triangulate();

	// Create the mesh
	DrawMesh mesh;
	mesh.points = inputPoints;

	// Convert the triangles
	auto triangles = cdt.GetTriangles();
	mesh.indices.reserve(triangles.size() * 3);
	auto firstPt = p2tPoints.data();
	for (const auto triangle : triangles)
	{
		mesh.indices.push_back(std::distance(firstPt, triangle->GetPoint(0)));
		mesh.indices.push_back(std::distance(firstPt, triangle->GetPoint(1)));
		mesh.indices.push_back(std::distance(firstPt, triangle->GetPoint(2)));
	}
	
	return mesh;
}

} // namespace graphics

} // namespace graphview

} // namespace panda
