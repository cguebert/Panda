#include <ui/graphview/graphics/DrawPath.h>

#include <algorithm>
#include <cassert>

using panda::types::Point;

namespace
{

	static void pathBezierToCasteljau(std::vector<Point>& path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
	{
		float dx = x4 - x1;
		float dy = y4 - y1;
		float d2 = ((x2 - x4) * dy - (y2 - y4) * dx); 
		float d3 = ((x3 - x4) * dy - (y3 - y4) * dx); 
		d2 = (d2 >= 0) ? d2 : -d2;
		d3 = (d3 >= 0) ? d3 : -d3;
		if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy)) 
		{
			path.push_back(Point(x4, y4));
		}
		else if (level < 10)
		{
			float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
			float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
			float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
			float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
			float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
			float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

			pathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1); 
			pathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1); 
		}
	}

}

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

void DrawPath::bezierCurveTo(const Point& p2, const Point& p3, const Point& p4, int num_segments)
{
	Point p1 = m_points.back();
	if (num_segments == 0)
	{
		// Auto-tessellated
		const float curveTessellationTol = 1.25f;
		pathBezierToCasteljau(m_points, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, curveTessellationTol, 0);
	}
	else
	{
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
		{
			float t = t_step * i_step;
			float u = 1.0f - t;
			float w1 = u*u*u;
			float w2 = 3*u*u*t;
			float w3 = 3*u*t*t;
			float w4 = t*t*t;
			m_points.push_back(Point(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
		}
	}
}

void DrawPath::rect(const Point& a, const Point& b, float rounding, int rounding_corners)
{
	float r = rounding;
	r = std::min(r, fabsf(b.x-a.x) * ( ((rounding_corners&(1|2))==(1|2)) || ((rounding_corners&(4|8))==(4|8)) ? 0.5f : 1.0f ) - 1.0f);
	r = std::min(r, fabsf(b.y-a.y) * ( ((rounding_corners&(1|8))==(1|8)) || ((rounding_corners&(2|4))==(2|4)) ? 0.5f : 1.0f ) - 1.0f);

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
