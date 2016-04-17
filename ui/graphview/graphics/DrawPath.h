#pragma once

#include <panda/types/Point.h>

#include <vector>

using pPoint = panda::types::Point;

class DrawPath
{
public:
	inline void clear() { m_points.clear(); }
	inline void lineTo(const pPoint& pos){ m_points.push_back(pos); }
	inline void moveTo(const pPoint& pos) { if (m_points.empty() || m_points.back() != pos) m_points.push_back(pos); }
	inline void close() { if (!m_points.empty() && m_points.back() != m_points.front()) m_points.push_back(m_points.front()); }
	void arcTo(const pPoint& centre, float radius, float a_min, float a_max, int num_segments = 10);
	void arcToFast(const pPoint& centre, float radius, int a_min_of_12, int a_max_of_12);                 // Use precomputed angles for a 12 steps circle
	void bezierCurveTo(const pPoint& p1, const pPoint& p2, const pPoint& p3, int num_segments = 0);
	void rect(const pPoint& rect_min, const pPoint& rect_max, float rounding = 0.0f, int rounding_corners = 0x0F);

	inline const std::vector<pPoint>& points() const { return m_points; }

	inline void translate(const pPoint& delta) { for (auto& pt : m_points) pt += delta; }
	bool contains(const pPoint& pos);

private:  
	std::vector<pPoint> m_points;
};
