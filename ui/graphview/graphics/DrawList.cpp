#include <ui/graphview/graphics/DrawList.h>
#include <ui/graphview/ViewRenderer.h>

#include <algorithm>
#include <cassert>

using panda::types::Point;

DrawList::DrawList()
{
	m_textureIdStack.push_back(ViewRenderer::defaultTextureId());
	addDrawCmd();
}

void DrawList::addDrawCmd()
{
	DrawCmd draw_cmd;
	draw_cmd.textureId = m_textureIdStack.empty() ? 0 : m_textureIdStack.back();

	m_cmdBuffer.push_back(draw_cmd);
}

void DrawList::updateTextureID()
{
	// If current command is used with different settings we need to add a new command
	const unsigned int curr_texture_id = m_textureIdStack.empty() ? 0 : m_textureIdStack.back();
	DrawCmd* curr_cmd = m_cmdBuffer.empty() ? nullptr : &m_cmdBuffer.back();
	if (!curr_cmd || (curr_cmd->elemCount != 0 && curr_cmd->textureId != curr_texture_id))
	{
		addDrawCmd();
		return;
	}
 
	// Try to merge with previous command if it matches, else use current command
	DrawCmd* prev_cmd = m_cmdBuffer.size() > 1 ? curr_cmd - 1 : nullptr;
	if (prev_cmd && prev_cmd->textureId == curr_texture_id)
		m_cmdBuffer.pop_back();
	else
		curr_cmd->textureId = curr_texture_id;
}

void DrawList::pushTextureID(unsigned int texture_id)
{
	m_textureIdStack.push_back(texture_id);
	updateTextureID();
}

void DrawList::popTextureID()
{
	assert(!m_textureIdStack.empty());
	m_textureIdStack.pop_back();
	updateTextureID();
}

void DrawList::addLine(const Point& a, const Point& b, unsigned int col, float thickness)
{
	if ((col >> 24) == 0)
		return;
	pathLineTo(a + Point(0.5f,0.5f));
	pathLineTo(b + Point(0.5f,0.5f));
	pathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void DrawList::addRect(const Point& a, const Point& b, unsigned int col, float rounding, int rounding_corners)
{
	if ((col >> 24) == 0)
		return;
	pathRect(a + Point(0.5f,0.5f), b - Point(0.5f,0.5f), rounding, rounding_corners);
	pathStroke(col, true);
}

void DrawList::addRectFilled(const Point& a, const Point& b, unsigned int col, float rounding, int rounding_corners)
{
	if ((col >> 24) == 0)
		return;
	if (rounding > 0.0f)
	{
		pathRect(a, b, rounding, rounding_corners);
		pathFill(col);
	}
	else
	{
		primReserve(6, 4);
		primRect(a, b, col);
	}
}

void DrawList::addPolyline(const std::vector<Point>& points, unsigned int col, bool closed, float thickness, bool anti_aliased)
{
	if (points.size() < 2)
		return;

	const Point uv(0, 0);

	int points_count = points.size();
	int count = points_count;
	if (!closed)
		count = points_count - 1;

	const bool thick_line = thickness > 1.0f;
	if (anti_aliased)
	{
		// Anti-aliased stroke
		const float AA_SIZE = 1.0f;
		const unsigned int col_trans = col & 0x00ffffff;

		const int idx_count = thick_line ? count*18 : count*12;
		const int vtx_count = thick_line ? points_count*4 : points_count*3;
		primReserve(idx_count, vtx_count);

		// Temporary buffer
		Point* temp_normals = (Point*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(Point));
		Point* temp_points = temp_normals + points_count;

		for (int i1 = 0; i1 < count; i1++)
		{
			const int i2 = (i1+1) == points_count ? 0 : i1+1;
			Point diff = points[i2] - points[i1];
			diff.normalize();
			temp_normals[i1].x = diff.y;
			temp_normals[i1].y = -diff.x;
		}
		if (!closed)
			temp_normals[points_count-1] = temp_normals[points_count-2];

		if (!thick_line)
		{
			if (!closed)
			{
				temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
				temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
				temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
				temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
			}

			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			DrawIdx idx1 = m_vtxCurrentIdx;
			for (int i1 = 0; i1 < count; i1++)
			{
				const int i2 = (i1+1) == points_count ? 0 : i1+1;
				DrawIdx idx2 = (i1+1) == points_count ? m_vtxCurrentIdx : idx1+3;

				// Average normals
				Point dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x*dm.x + dm.y*dm.y;
				if (dmr2 > 0.000001f)
				{
					float scale = 1.0f / dmr2;
					if (scale > 100.0f) scale = 100.0f;
					dm *= scale;
				}
				dm *= AA_SIZE;
				temp_points[i2*2+0] = points[i2] + dm;
				temp_points[i2*2+1] = points[i2] - dm;

				// Add indexes
				m_idxWritePtr[0] = idx2+0; m_idxWritePtr[1] = idx1+0; m_idxWritePtr[2] = idx1+2;
				m_idxWritePtr[3] = idx1+2; m_idxWritePtr[4] = idx2+2; m_idxWritePtr[5] = idx2+0;
				m_idxWritePtr[6] = idx2+1; m_idxWritePtr[7] = idx1+1; m_idxWritePtr[8] = idx1+0;
				m_idxWritePtr[9] = idx1+0; m_idxWritePtr[10]= idx2+0; m_idxWritePtr[11]= idx2+1;
				m_idxWritePtr += 12;

				idx1 = idx2;
			}

			// Add vertexes
			for (int i = 0; i < points_count; i++)
			{
				m_vtxWritePtr[0].pos = points[i];          m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col;
				m_vtxWritePtr[1].pos = temp_points[i*2+0]; m_vtxWritePtr[1].uv = uv; m_vtxWritePtr[1].col = col_trans;
				m_vtxWritePtr[2].pos = temp_points[i*2+1]; m_vtxWritePtr[2].uv = uv; m_vtxWritePtr[2].col = col_trans;
				m_vtxWritePtr += 3;
			}
		}
		else
		{
			const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
			if (!closed)
			{
				temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
				temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
				temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
				temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
				temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
				temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
			}

			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			DrawIdx idx1 = m_vtxCurrentIdx;
			for (int i1 = 0; i1 < count; i1++)
			{
				const int i2 = (i1+1) == points_count ? 0 : i1+1;
				DrawIdx idx2 = (i1+1) == points_count ? m_vtxCurrentIdx : idx1+4;

				// Average normals
				Point dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
				float dmr2 = dm.x*dm.x + dm.y*dm.y;
				if (dmr2 > 0.000001f)
				{
					float scale = 1.0f / dmr2;
					if (scale > 100.0f) scale = 100.0f;
					dm *= scale;
				}
				Point dm_out = dm * (half_inner_thickness + AA_SIZE);
				Point dm_in = dm * half_inner_thickness;
				temp_points[i2*4+0] = points[i2] + dm_out;
				temp_points[i2*4+1] = points[i2] + dm_in;
				temp_points[i2*4+2] = points[i2] - dm_in;
				temp_points[i2*4+3] = points[i2] - dm_out;

				// Add indexes
				m_idxWritePtr[0]  = idx2+1; m_idxWritePtr[1]  = idx1+1; m_idxWritePtr[2]  = idx1+2;
				m_idxWritePtr[3]  = idx1+2; m_idxWritePtr[4]  = idx2+2; m_idxWritePtr[5]  = idx2+1;
				m_idxWritePtr[6]  = idx2+1; m_idxWritePtr[7]  = idx1+1; m_idxWritePtr[8]  = idx1+0;
				m_idxWritePtr[9]  = idx1+0; m_idxWritePtr[10] = idx2+0; m_idxWritePtr[11] = idx2+1;
				m_idxWritePtr[12] = idx2+2; m_idxWritePtr[13] = idx1+2; m_idxWritePtr[14] = idx1+3;
				m_idxWritePtr[15] = idx1+3; m_idxWritePtr[16] = idx2+3; m_idxWritePtr[17] = idx2+2;
				m_idxWritePtr += 18;

				idx1 = idx2;
			}

			// Add vertexes
			for (int i = 0; i < points_count; i++)
			{
				m_vtxWritePtr[0].pos = temp_points[i*4+0]; m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col_trans;
				m_vtxWritePtr[1].pos = temp_points[i*4+1]; m_vtxWritePtr[1].uv = uv; m_vtxWritePtr[1].col = col;
				m_vtxWritePtr[2].pos = temp_points[i*4+2]; m_vtxWritePtr[2].uv = uv; m_vtxWritePtr[2].col = col;
				m_vtxWritePtr[3].pos = temp_points[i*4+3]; m_vtxWritePtr[3].uv = uv; m_vtxWritePtr[3].col = col_trans;
				m_vtxWritePtr += 4;
			}
		}
		m_vtxCurrentIdx += vtx_count;
	}
	else
	{
		// Non Anti-aliased Stroke
		const int idx_count = count*6;
		const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
		primReserve(idx_count, vtx_count);

		for (int i1 = 0; i1 < count; i1++)
		{
			const int i2 = (i1+1) == points_count ? 0 : i1+1;
			const Point& p1 = points[i1];
			const Point& p2 = points[i2];
			Point diff = p2 - p1;
			diff.normalize();

			const float dx = diff.x * (thickness * 0.5f);
			const float dy = diff.y * (thickness * 0.5f);
			m_vtxWritePtr[0].pos.x = p1.x + dy; m_vtxWritePtr[0].pos.y = p1.y - dx; m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col;
			m_vtxWritePtr[1].pos.x = p2.x + dy; m_vtxWritePtr[1].pos.y = p2.y - dx; m_vtxWritePtr[1].uv = uv; m_vtxWritePtr[1].col = col;
			m_vtxWritePtr[2].pos.x = p2.x - dy; m_vtxWritePtr[2].pos.y = p2.y + dx; m_vtxWritePtr[2].uv = uv; m_vtxWritePtr[2].col = col;
			m_vtxWritePtr[3].pos.x = p1.x - dy; m_vtxWritePtr[3].pos.y = p1.y + dx; m_vtxWritePtr[3].uv = uv; m_vtxWritePtr[3].col = col;
			m_vtxWritePtr += 4;

			m_idxWritePtr[0] = m_vtxCurrentIdx; m_idxWritePtr[1] = m_vtxCurrentIdx+1; m_idxWritePtr[2] = m_vtxCurrentIdx+2;
			m_idxWritePtr[3] = m_vtxCurrentIdx; m_idxWritePtr[4] = m_vtxCurrentIdx+2; m_idxWritePtr[5] = m_vtxCurrentIdx+3;
			m_idxWritePtr += 6;
			m_vtxCurrentIdx += 4;
		}
	}
}

void DrawList::addConvexPolyFilled(const std::vector<Point>& points, unsigned int col, bool anti_aliased)
{
	const Point uv(0, 0);
	int points_count = points.size();

	if (anti_aliased)
	{
		// Anti-aliased Fill
		const float AA_SIZE = 1.0f;
		const unsigned int col_trans = col & 0x00ffffff;
		const int idx_count = (points_count-2)*3 + points_count*6;
		const int vtx_count = (points_count*2);
		primReserve(idx_count, vtx_count);

		// Add indexes for fill
		unsigned int vtx_inner_idx = m_vtxCurrentIdx;
		unsigned int vtx_outer_idx = m_vtxCurrentIdx+1;
		for (int i = 2; i < points_count; i++)
		{
			m_idxWritePtr[0] = vtx_inner_idx; m_idxWritePtr[1] = vtx_inner_idx+((i-1)<<1); m_idxWritePtr[2] = vtx_inner_idx+(i<<1);
			m_idxWritePtr += 3;
		}

		// Compute normals
		Point* temp_normals = (Point*)alloca(points_count * sizeof(Point));
		for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
		{
			const Point& p0 = points[i0];
			const Point& p1 = points[i1];
			Point diff = p1 - p0;
			diff.normalize();
			temp_normals[i0].x = diff.y;
			temp_normals[i0].y = -diff.x;
		}

		for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
		{
			// Average normals
			const Point& n0 = temp_normals[i0];
			const Point& n1 = temp_normals[i1];
			Point dm = (n0 + n1) * 0.5f;
			float dmr2 = dm.x*dm.x + dm.y*dm.y;
			if (dmr2 > 0.000001f)
			{
				float scale = 1.0f / dmr2;
				if (scale > 100.0f) scale = 100.0f;
				dm *= scale;
			}
			dm *= AA_SIZE * 0.5f;

			// Add vertices
			m_vtxWritePtr[0].pos = (points[i1] - dm); m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col;        // Inner
			m_vtxWritePtr[1].pos = (points[i1] + dm); m_vtxWritePtr[1].uv = uv; m_vtxWritePtr[1].col = col_trans;  // Outer
			m_vtxWritePtr += 2;

			// Add indexes for fringes
			m_idxWritePtr[0] = vtx_inner_idx+(i1<<1); m_idxWritePtr[1] = vtx_inner_idx+(i0<<1); m_idxWritePtr[2] = vtx_outer_idx+(i0<<1);
			m_idxWritePtr[3] = vtx_outer_idx+(i0<<1); m_idxWritePtr[4] = vtx_outer_idx+(i1<<1); m_idxWritePtr[5] = vtx_inner_idx+(i1<<1);
			m_idxWritePtr += 6;
		}
		m_vtxCurrentIdx += vtx_count;
	}
	else
	{
		// Non Anti-aliased Fill
		const int idx_count = (points_count-2)*3;
		const int vtx_count = points_count;
		primReserve(idx_count, vtx_count);
		for (int i = 0; i < vtx_count; i++)
		{
			m_vtxWritePtr[0].pos = points[i]; m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col;
			m_vtxWritePtr++;
		}
		for (int i = 2; i < points_count; i++)
		{
			m_idxWritePtr[0] = m_vtxCurrentIdx; m_idxWritePtr[1] = m_vtxCurrentIdx+i-1; m_idxWritePtr[2] = m_vtxCurrentIdx+i;
			m_idxWritePtr += 3;
		}
		m_vtxCurrentIdx += vtx_count;
	}
}

void DrawList::pathArcToFast(const Point& centre, float radius, int amin, int amax)
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
		m_path.push_back(centre);
	}
	else
	{
		m_path.reserve(m_path.size() + (amax - amin + 1));
		for (int a = amin; a <= amax; a++)
		{
			const Point& c = circle_vtx[a % circle_vtx_count];
			m_path.push_back(Point(centre.x + c.x * radius, centre.y + c.y * radius));
		}
	}
}

void DrawList::pathArcTo(const Point& centre, float radius, float amin, float amax, int num_segments)
{
	if (radius == 0.0f)
		m_path.push_back(centre);
	m_path.reserve(m_path.size() + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = amin + ((float)i / (float)num_segments) * (amax - amin);
		m_path.push_back(Point(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
	}
}

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

void DrawList::pathBezierCurveTo(const Point& p2, const Point& p3, const Point& p4, int num_segments)
{
	Point p1 = m_path.back();
	if (num_segments == 0)
	{
		// Auto-tessellated
		const float curveTessellationTol = 1.25f;
		pathBezierToCasteljau(m_path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, curveTessellationTol, 0);
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
			m_path.push_back(Point(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
		}
	}
}

void DrawList::pathRect(const Point& a, const Point& b, float rounding, int rounding_corners)
{
	float r = rounding;
	r = std::min(r, fabsf(b.x-a.x) * ( ((rounding_corners&(1|2))==(1|2)) || ((rounding_corners&(4|8))==(4|8)) ? 0.5f : 1.0f ) - 1.0f);
	r = std::min(r, fabsf(b.y-a.y) * ( ((rounding_corners&(1|8))==(1|8)) || ((rounding_corners&(2|4))==(2|4)) ? 0.5f : 1.0f ) - 1.0f);

	if (r <= 0.0f || rounding_corners == 0)
	{
		pathLineTo(a);
		pathLineTo(Point(b.x,a.y));
		pathLineTo(b);
		pathLineTo(Point(a.x,b.y));
	}
	else
	{
		const float r0 = (rounding_corners & 1) ? r : 0.0f;
		const float r1 = (rounding_corners & 2) ? r : 0.0f;
		const float r2 = (rounding_corners & 4) ? r : 0.0f;
		const float r3 = (rounding_corners & 8) ? r : 0.0f;
		pathArcToFast(Point(a.x+r0,a.y+r0), r0, 6, 9);
		pathArcToFast(Point(b.x-r1,a.y+r1), r1, 9, 12);
		pathArcToFast(Point(b.x-r2,b.y-r2), r2, 0, 3);
		pathArcToFast(Point(a.x+r3,b.y-r3), r3, 3, 6);
	}
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void DrawList::primReserve(int idx_count, int vtx_count)
{
	DrawCmd& draw_cmd = m_cmdBuffer.back();
	draw_cmd.elemCount += idx_count;

	int vtx_buffer_size = m_vtxBuffer.size();
	m_vtxBuffer.resize(vtx_buffer_size + vtx_count);
	m_vtxWritePtr = &m_vtxBuffer[vtx_buffer_size];

	int idx_buffer_size = m_idxBuffer.size();
	m_idxBuffer.resize(idx_buffer_size + idx_count);
	m_idxWritePtr = &m_idxBuffer[idx_buffer_size];
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void DrawList::primRect(const Point& a, const Point& c, unsigned int col)
{
	Point b(c.x, a.y), d(a.x, c.y), uv(0, 0);
	DrawIdx idx = (DrawIdx)m_vtxCurrentIdx;
	m_idxWritePtr[0] = idx; m_idxWritePtr[1] = (DrawIdx)(idx+1); m_idxWritePtr[2] = (DrawIdx)(idx+2);
	m_idxWritePtr[3] = idx; m_idxWritePtr[4] = (DrawIdx)(idx+2); m_idxWritePtr[5] = (DrawIdx)(idx+3);
	m_vtxWritePtr[0].pos = a; m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col;
	m_vtxWritePtr[1].pos = b; m_vtxWritePtr[1].uv = uv; m_vtxWritePtr[1].col = col;
	m_vtxWritePtr[2].pos = c; m_vtxWritePtr[2].uv = uv; m_vtxWritePtr[2].col = col;
	m_vtxWritePtr[3].pos = d; m_vtxWritePtr[3].uv = uv; m_vtxWritePtr[3].col = col;
	m_vtxWritePtr += 4;
	m_vtxCurrentIdx += 4;
	m_idxWritePtr += 6;
}