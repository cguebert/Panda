#include <ui/graphview/graphics/DrawList.h>
#include <ui/graphview/graphics/FontAtlas.h>
#include <ui/graphview/ViewRenderer.h>

#include <QColor>

#include <algorithm>
#include <cassert>

using panda::types::Point;
using panda::types::Rect;

namespace
{
	inline Rect defaultClipRect()
	{
		return Rect(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);
	}
}

namespace graphview
{

namespace graphics
{

unsigned int DrawList::convert(const QColor& col)
{
	unsigned int out;
	out = col.red() & 0xFF;
	out |= (col.green() & 0xFF) << 8;
	out |= (col.blue() & 0xFF) << 16;
	out |= (col.alpha() & 0xFF) << 24;
	return out;
}

DrawList::DrawList()
{
	if (ViewRenderer::initialized())
	{
		m_textureIdStack.push_back(ViewRenderer::defaultTextureId());
		m_clipRectStack.push_back(defaultClipRect());
	}
	addDrawCmd();
}

void DrawList::clear()
{
	m_cmdBuffer.clear();
	m_vtxBuffer.clear();
	m_idxBuffer.clear();
	m_vtxCurrentIdx = 0;
	m_vtxWritePtr = nullptr;
	m_idxWritePtr = nullptr;

	addDrawCmd();
}

void DrawList::addDrawCmd()
{
	DrawCmd draw_cmd;
	draw_cmd.textureId = m_textureIdStack.empty() ? 0 : m_textureIdStack.back();
	draw_cmd.clipRect = m_clipRectStack.empty() ? defaultClipRect() : m_clipRectStack.back();

	m_cmdBuffer.push_back(draw_cmd);
}

void DrawList::merge(DrawList& list, bool ignoreClip)
{
	if (list.m_cmdBuffer.empty() || list.m_idxBuffer.empty())
		return;

	const auto& listFront = list.m_cmdBuffer.front();
	const auto& thisBack = m_cmdBuffer.back();
	if (m_cmdBuffer.empty()
		|| (!ignoreClip && (listFront.clipRect != thisBack.clipRect))
		|| listFront.textureId != thisBack.textureId)
		addDrawCmd();

	auto lastIdx = m_idxBuffer.size();
	auto lastVtx = m_vtxBuffer.size();
	m_idxBuffer.insert(m_idxBuffer.end(), list.m_idxBuffer.begin(), list.m_idxBuffer.end());
	m_vtxBuffer.insert(m_vtxBuffer.end(), list.m_vtxBuffer.begin(), list.m_vtxBuffer.end());

	// Update the indices we inserted
	std::for_each(m_idxBuffer.begin() + lastIdx, m_idxBuffer.end(), [lastVtx](auto& v) { v += lastVtx; });

	// Insert the commands
	m_cmdBuffer.back().elemCount += list.m_cmdBuffer.front().elemCount;
	for (int i = 1, nb = list.m_cmdBuffer.size(); i < nb; ++i)
		m_cmdBuffer.push_back(list.m_cmdBuffer[i]);

	m_vtxCurrentIdx = m_vtxBuffer.size();
}

void DrawList::updateClipRect()
{
	// If current command is used with different settings we need to add a new command
	const Rect curr_clip_rect = m_clipRectStack.empty() ? defaultClipRect() : m_clipRectStack.back();
	DrawCmd* curr_cmd = m_cmdBuffer.empty() ? nullptr : &m_cmdBuffer.back();
	if (!curr_cmd || (curr_cmd->elemCount != 0 && curr_cmd->clipRect != curr_clip_rect))
	{
		addDrawCmd();
		return;
	}

	// Try to merge with previous command if it matches, else use current command
	DrawCmd* prev_cmd = m_cmdBuffer.size() > 1 ? curr_cmd - 1 : nullptr;
	if (prev_cmd && curr_cmd->clipRect == curr_clip_rect)
		m_cmdBuffer.pop_back();
	else
		curr_cmd->clipRect = curr_clip_rect;
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

void DrawList::pushClipRect(const pRect& clip_rect)
{
	m_clipRectStack.push_back(clip_rect);
	updateClipRect();
}

void DrawList::popClipRect()
{
	assert(!m_clipRectStack.empty());
	m_clipRectStack.pop_back();
	updateClipRect();
}

void DrawList::addLine(const Point& a, const Point& b, unsigned int col, float thickness)
{
	if ((col >> 24) == 0)
		return;
	m_path.lineTo(a + Point(0.5f,0.5f));
	m_path.lineTo(b + Point(0.5f,0.5f));
	pathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void DrawList::addRect(const Rect& r, unsigned int col, float thickness, float rounding, int rounding_corners)
{
	if ((col >> 24) == 0)
		return;
	m_path.rect(r, rounding, rounding_corners);
	pathStroke(col, true, thickness);
}

void DrawList::addRectFilled(const Rect& r, unsigned int col, float rounding, int rounding_corners)
{
	if ((col >> 24) == 0)
		return;
	if (rounding > 0.0f)
	{
		m_path.rect(r, rounding, rounding_corners);
		pathFill(col);
	}
	else
	{
		primReserve(6, 4);
		primRect(r, col);
	}
}

void DrawList::addRectFilledMultiColor(const Rect& r, unsigned int col_upr_left, unsigned int col_upr_right, unsigned int col_bot_right, unsigned int col_bot_left)
{
	if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) >> 24) == 0)
		return;

	const Point uv(0, 0);
	primReserve(6, 4);
	primWriteIdx(m_vtxCurrentIdx); primWriteIdx(m_vtxCurrentIdx+1); primWriteIdx(m_vtxCurrentIdx+2);
	primWriteIdx(m_vtxCurrentIdx); primWriteIdx(m_vtxCurrentIdx+2); primWriteIdx(m_vtxCurrentIdx+3);
	primWriteVtx(r.topLeft(), uv, col_upr_left);
	primWriteVtx(r.topRight(), uv, col_upr_right);
	primWriteVtx(r.bottomRight(), uv, col_bot_right);
	primWriteVtx(r.bottomLeft(), uv, col_bot_left);
}

void DrawList::addTriangle(const Point& a, const Point& b, const Point& c, unsigned int col, float thickness)
{
	if ((col >> 24) == 0)
		return;

	m_path.lineTo(a);
	m_path.lineTo(b);
	m_path.lineTo(c);
	pathStroke(col, true, thickness);
}

void DrawList::addTriangleFilled(const Point& a, const Point& b, const Point& c, unsigned int col)
{
	if ((col >> 24) == 0)
		return;

	m_path.lineTo(a);
	m_path.lineTo(b);
	m_path.lineTo(c);
	pathFill(col);
}

void DrawList::addCircle(const Point& centre, float radius, unsigned int col, int num_segments, float thickness)
{
	if ((col >> 24) == 0)
		return;

	const float a_max = M_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
	m_path.arcTo(centre, radius-0.5f, 0.0f, a_max, num_segments);
	pathStroke(col, true, thickness);
}

void DrawList::addCircleFilled(const Point& centre, float radius, unsigned int col, int num_segments)
{
	if ((col >> 24) == 0)
		return;

	const float a_max = M_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
	m_path.arcTo(centre, radius, 0.0f, a_max, num_segments);
	pathFill(col);
}

void DrawList::addEllipse(const pRect& rect, unsigned int col, float thickness, float precision)
{
	if ((col >> 24) == 0)
		return;

	m_path.arcTo(rect, 0, (float)M_PI * 2.0f, precision);
	pathStroke(col, false, thickness);
}

void DrawList::addEllipseFilled(const pRect& rect, unsigned int col, float precision)
{
	if ((col >> 24) == 0)
		return;

	m_path.arcTo(rect, 0, (float)M_PI * 2.0f, precision);
	auto mesh = m_path.triangulate();
	m_path.clear();
	addMesh(mesh, col);
}

void DrawList::addBezierCurve(const Point& pos0, const Point& cp0, const Point& cp1, const Point& pos1, unsigned int col, float thickness, int num_segments)
{
	if ((col >> 24) == 0)
		return;

	m_path.lineTo(pos0);
	m_path.bezierCurveTo(cp0, cp1, pos1, num_segments);
	pathStroke(col, false, thickness);
}

void DrawList::addText(const Font& font, const Point& pos, const std::string& text, unsigned int col, float font_scale, float wrap_width, const Rect* cpu_fine_clip_rect)
{
	if ((col >> 24) == 0)
		return;

	if (text.empty())
		return;

	assert(font.atlas()->texID() == m_textureIdStack.back());  // Use high-level ImGui::PushFont() or low-level DrawList::PushTextureId() to change font.

	// reserve vertices for worse case (over-reserving is useful and easily amortized)
	const int char_count = text.size();
	const int vtx_count_max = char_count * 4;
	const int idx_count_max = char_count * 6;
	const int vtx_begin = m_vtxBuffer.size();
	const int idx_begin = m_idxBuffer.size();
	primReserve(idx_count_max, vtx_count_max);

	Rect clip_rect = m_clipRectStack.back();
	if (cpu_fine_clip_rect)
	{
		clip_rect.setLeft(std::max(clip_rect.left(), cpu_fine_clip_rect->left()));
		clip_rect.setTop(std::max(clip_rect.top(), cpu_fine_clip_rect->top()));
		clip_rect.setRight(std::max(clip_rect.right(), cpu_fine_clip_rect->right()));
		clip_rect.setBottom(std::max(clip_rect.bottom(), cpu_fine_clip_rect->bottom()));
	}
	font.renderText(font_scale, pos, col, clip_rect, text, this, wrap_width, cpu_fine_clip_rect != NULL);

	// give back unused vertices
	// FIXME-OPT: clean this up
	m_vtxBuffer.resize((int)(m_vtxWritePtr - m_vtxBuffer.data()));
	m_idxBuffer.resize((int)(m_idxWritePtr - m_idxBuffer.data()));
	int vtx_unused = vtx_count_max - (m_vtxBuffer.size() - vtx_begin);
	int idx_unused = idx_count_max - (m_idxBuffer.size() - idx_begin);
	m_cmdBuffer.back().elemCount -= idx_unused;
	m_vtxWritePtr -= vtx_unused;
	m_idxWritePtr -= idx_unused;
	m_vtxCurrentIdx = m_vtxBuffer.size();
}

void DrawList::addText(const Point& pos, const std::string& text, unsigned int col)
{
	addText(*ViewRenderer::currentFont(), pos, text, col);
}

void DrawList::addText(const Rect& rect, const std::string& text, unsigned int col, int align, float scale, bool wrap, bool fit)
{
	if (text.empty() || rect.empty())
		return;

	auto font = ViewRenderer::currentFont();
	if (!font)
		return;

	// Perform CPU side clipping for single clipped element to avoid using scissor state
	float wrap_width = wrap ? rect.width() : 0.0f;
	Point text_size = font->calcTextSize(scale, wrap_width, text, false); // Do not cut words

	// If the text is too big to fit, we scale down the font
	if (fit && text_size.x > rect.width())
	{
		scale = rect.width() / (text_size.x + 1.f);
		text_size *= scale;
	}
	if (fit)
	{
		while (text_size.y > rect.height())
		{
			scale -= 0.05f;
			text_size = font->calcTextSize(scale, wrap_width, text, false);
		}
	}
	
	// Align
	Point pos = rect.topLeft();
	if (align & Align_HCenter) pos.x = std::max(pos.x, (pos.x + rect.right() - text_size.x) * 0.5f);
	else if (align & Align_Right) pos.x = std::max(pos.x, rect.right() - text_size.x);

	if (align & Align_VCenter) pos.y = std::max(pos.y, (pos.y + rect.bottom() - text_size.y) * 0.5f);
	else if(align & Align_Bottom) pos.y = std::max(pos.y, rect.bottom() - text_size.y);

	// Render
	bool need_clipping = (rect.left() + text_size.x >= rect.right()) || (rect.top() + text_size.y >= rect.bottom());
	if (need_clipping)
		addText(*font, pos, text, col, scale, wrap_width, &rect);
	else
		addText(*font, pos, text, col, scale, wrap_width, nullptr);
}

void DrawList::addImage(unsigned int user_texture_id, const Rect& r, const Rect& uv, unsigned int col)
{
	if ((col >> 24) == 0)
		return;

	// FIXME-OPT: This is wasting draw calls.
	const bool push_texture_id = m_textureIdStack.empty() || user_texture_id != m_textureIdStack.back();
	if (push_texture_id)
		pushTextureID(user_texture_id);

	primReserve(6, 4);
	primRectUV(r, uv, col);

	if (push_texture_id)
		popTextureID();
}

void DrawList::addPolyline(const DrawPath& path, unsigned int col, bool close, float thickness, bool anti_aliased)
{
	const auto& points = path.points();
	int points_count = points.size();
	if (points_count < 2)
		return;

	const Point uv(0, 0);
	int count = points_count;
	if (!close)
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
		if (!close)
			temp_normals[points_count-1] = temp_normals[points_count-2];

		if (!thick_line)
		{
			if (!close)
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
			if (!close)
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

void DrawList::addConvexPolyFilled(const DrawPath& path, unsigned int col, bool anti_aliased)
{
	const auto& points = path.points();
	int points_count = points.size();
	if (points_count < 2)
		return;

	const Point uv(0, 0);

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

void DrawList::addMesh(const DrawMesh& mesh, unsigned int col)
{
	const auto& points = mesh.points;
	int points_count = points.size();
	if (points_count < 2)
		return;

	const Point uv(0, 0);

	// Non Anti-aliased Fill
	const int idx_count = mesh.indices.size();
	const int vtx_count = points_count;
	primReserve(idx_count, vtx_count);
	for (int i = 0; i < vtx_count; i++)
	{
		m_vtxWritePtr->pos = points[i]; m_vtxWritePtr->uv = uv; m_vtxWritePtr->col = col;
		m_vtxWritePtr++;
	}

	for (int i = 0; i < idx_count; i++)
		*m_idxWritePtr++ = mesh.indices[i] + m_vtxCurrentIdx;
	
	m_vtxCurrentIdx += vtx_count;
}

Point DrawList::calcTextSize(float scale, const std::string& text, float wrap_width, bool cutWords)
{
	auto font = ViewRenderer::currentFont();
	if (!font)
		return Point();

	return font->calcTextSize(scale, wrap_width, text, cutWords); // Do not cut words
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void DrawList::primReserve(int idx_count, int vtx_count)
{
	DrawCmd& draw_cmd = m_cmdBuffer.back();
	draw_cmd.elemCount += idx_count;

	int vtx_buffer_size = m_vtxBuffer.size();
	m_vtxBuffer.resize(vtx_buffer_size + vtx_count);
	if (vtx_count)
		m_vtxWritePtr = &m_vtxBuffer[vtx_buffer_size];

	int idx_buffer_size = m_idxBuffer.size();
	m_idxBuffer.resize(idx_buffer_size + idx_count);
	if (idx_count)
		m_idxWritePtr = &m_idxBuffer[idx_buffer_size];
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void DrawList::primRect(const Rect& r, unsigned int col)
{
	Point uv(0, 0);
	DrawIdx idx = (DrawIdx)m_vtxCurrentIdx;
	m_idxWritePtr[0] = idx; m_idxWritePtr[1] = (DrawIdx)(idx+1); m_idxWritePtr[2] = (DrawIdx)(idx+2);
	m_idxWritePtr[3] = idx; m_idxWritePtr[4] = (DrawIdx)(idx+2); m_idxWritePtr[5] = (DrawIdx)(idx+3);
	m_vtxWritePtr[0].pos = r.topLeft();		m_vtxWritePtr[0].uv = uv; m_vtxWritePtr[0].col = col;
	m_vtxWritePtr[1].pos = r.topRight();	m_vtxWritePtr[1].uv = uv; m_vtxWritePtr[1].col = col;
	m_vtxWritePtr[2].pos = r.bottomRight(); m_vtxWritePtr[2].uv = uv; m_vtxWritePtr[2].col = col;
	m_vtxWritePtr[3].pos = r.bottomLeft();	m_vtxWritePtr[3].uv = uv; m_vtxWritePtr[3].col = col;
	m_vtxWritePtr += 4;
	m_vtxCurrentIdx += 4;
	m_idxWritePtr += 6;
}

void DrawList::primRectUV(const Rect& r, const Rect& uv, unsigned int col)
{
	DrawIdx idx = m_vtxCurrentIdx;
	m_idxWritePtr[0] = idx; m_idxWritePtr[1] = idx+1; m_idxWritePtr[2] = idx+2;
	m_idxWritePtr[3] = idx; m_idxWritePtr[4] = idx+2; m_idxWritePtr[5] = idx+3;
	m_vtxWritePtr[0].pos = r.topLeft();		m_vtxWritePtr[0].uv = uv.topLeft();		m_vtxWritePtr[0].col = col;
	m_vtxWritePtr[1].pos = r.topRight();	m_vtxWritePtr[1].uv = uv.topRight();	m_vtxWritePtr[1].col = col;
	m_vtxWritePtr[2].pos = r.bottomRight();	m_vtxWritePtr[2].uv = uv.bottomRight();	m_vtxWritePtr[2].col = col;
	m_vtxWritePtr[3].pos = r.bottomLeft();	m_vtxWritePtr[3].uv = uv.bottomLeft();	m_vtxWritePtr[3].col = col;
	m_vtxWritePtr += 4;
	m_vtxCurrentIdx += 4;
	m_idxWritePtr += 6;
}

} // namespace graphics

} // namespace graphview
