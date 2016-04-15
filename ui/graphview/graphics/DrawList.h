#pragma once

#include <panda/types/Rect.h>

#include <vector>

class QColor;
class Font;

struct DrawCmd
{
	int elemCount = 0; // Number of indices (multiple of 3) to be rendered as triangles.
	unsigned int textureId = 0; // User-provided texture ID
	panda::types::Rect clipRect; // Clipping rectangle (x1, y1, x2, y2)
};

using DrawIdx = unsigned int;

struct DrawVert
{
	panda::types::Point pos, uv;
	unsigned int col;
};

class DrawList
{
public:
	DrawList();

	static unsigned int convert(const QColor& col);

	void addDrawCmd(); // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
	void pushTextureID(unsigned int texture_id);
	void popTextureID();

	// Primitives
	void addLine(const panda::types::Point& a, const panda::types::Point& b, unsigned int col, float thickness = 1.0f);
	void addRect(const panda::types::Point& a, const panda::types::Point& b, unsigned int col, float rounding = 0.0f, int rounding_corners = 0x0F);        // a: upper-left, b: lower-right
	void addRectFilled(const panda::types::Point& a, const panda::types::Point& b, unsigned int col, float rounding = 0.0f, int rounding_corners = 0x0F);  // a: upper-left, b: lower-right
	void addRectFilledMultiColor(const panda::types::Point& a, const panda::types::Point& b, unsigned int col_upr_left, unsigned int col_upr_right, unsigned int col_bot_right, unsigned int col_bot_left);
	void addTriangle(const panda::types::Point& a, const panda::types::Point& b, const panda::types::Point& c, unsigned int col, float thickness);
	void addTriangleFilled(const panda::types::Point& a, const panda::types::Point& b, const panda::types::Point& c, unsigned int col);
	void addCircle(const panda::types::Point& centre, float radius, unsigned int col, int num_segments = 12, float thickness = 1.0f);
	void addCircleFilled(const panda::types::Point& centre, float radius, unsigned int col, int num_segments = 12);
	void addBezierCurve(const panda::types::Point& pos0, const panda::types::Point& cp0, const panda::types::Point& cp1, const panda::types::Point& pos1, unsigned int col, float thickness, int num_segments = 0);
	void addText(const panda::types::Point& pos, unsigned int col, const char* text_begin, const char* text_end = NULL);
	void addText(const Font& font, float font_scale, const panda::types::Point& pos, unsigned int col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const panda::types::Rect* cpu_fine_clip_rect = NULL);
	void addImage(unsigned int texture_id, const panda::types::Point& a, const panda::types::Point& b, const panda::types::Point& uv0 = panda::types::Point(0,0), const panda::types::Point& uv1 = panda::types::Point(1,1), unsigned int col = 0xFFFFFFFF);
	void addPolyline(const std::vector<panda::types::Point>& points, unsigned int col, bool closed, float thickness, bool anti_aliased);
	void addConvexPolyFilled(const std::vector<panda::types::Point>& points, unsigned int col, bool anti_aliased);

	// Stateful path API, add points then finish with PathFill() or PathStroke()
	inline void pathClear() { m_path.clear(); }
	inline void pathLineTo(const panda::types::Point& pos){ m_path.push_back(pos); }
	inline void pathLineToMergeDuplicate(const panda::types::Point& pos) { if (m_path.empty() || m_path.back().x != pos.x || m_path.back().y != pos.y) m_path.push_back(pos); }
	inline void pathFill(unsigned int col) { addConvexPolyFilled(m_path, col, true); pathClear(); }
	inline void pathStroke(unsigned int col, bool closed, float thickness = 1.0f) { addPolyline(m_path, col, closed, thickness, true); pathClear(); }
	void pathArcTo(const panda::types::Point& centre, float radius, float a_min, float a_max, int num_segments = 10);
	void pathArcToFast(const panda::types::Point& centre, float radius, int a_min_of_12, int a_max_of_12);                 // Use precomputed angles for a 12 steps circle
	void pathBezierCurveTo(const panda::types::Point& p1, const panda::types::Point& p2, const panda::types::Point& p3, int num_segments = 0);
	void pathRect(const panda::types::Point& rect_min, const panda::types::Point& rect_max, float rounding = 0.0f, int rounding_corners = 0x0F);

	inline const std::vector<DrawCmd>& cmdBuffer() const { return m_cmdBuffer; }
	inline const std::vector<DrawIdx>& idxBuffer() const { return m_idxBuffer; }
	inline const std::vector<DrawVert>& vtxBuffer() const { return m_vtxBuffer; }

	inline DrawIdx vtxCurrentIdx() const { return m_vtxCurrentIdx; }
	inline DrawVert* vtxWritePtr() const { return m_vtxWritePtr; }
	inline DrawIdx* idxWritePtr() const { return m_idxWritePtr; } 

	inline void setVtxCurrentIdx(DrawIdx idx) { m_vtxCurrentIdx = idx; }
	inline void setVtxWritePtr(DrawVert* ptr) { m_vtxWritePtr = ptr; }
	inline void setIdxWritePtr(DrawIdx* ptr) { m_idxWritePtr = ptr; }

private:  
	void primReserve(int idx_count, int vtx_count);
	void primRect(const panda::types::Point& a, const panda::types::Point& b, unsigned int col);      // Axis aligned rectangle (composed of two triangles)
	void primRectUV(const panda::types::Point& a, const panda::types::Point& b, const panda::types::Point& uv_a, const panda::types::Point& uv_b, unsigned int col);
	inline void primVtx(const panda::types::Point& pos, const panda::types::Point& uv, unsigned int col)
	{ primWriteIdx(m_vtxCurrentIdx); primWriteVtx(pos, uv, col); }
    inline void primWriteVtx(const panda::types::Point& pos, const panda::types::Point& uv, unsigned int col)
	{ m_vtxWritePtr->pos = pos; m_vtxWritePtr->uv = uv; m_vtxWritePtr->col = col; m_vtxWritePtr++; m_vtxCurrentIdx++; }
    inline void primWriteIdx(DrawIdx idx) { *m_idxWritePtr = idx; m_idxWritePtr++; }

	void updateTextureID();
	void updateClipRect();
	
	std::vector<DrawCmd> m_cmdBuffer; // Commands. Typically 1 command = 1 gpu draw call.
	std::vector<DrawIdx> m_idxBuffer; // Index buffer. Each command consume DrawCmd::ElemCount of those
	std::vector<DrawVert> m_vtxBuffer; // Vertex buffer.

	DrawIdx m_vtxCurrentIdx = 0; // VtxBuffer.Size
	DrawVert* m_vtxWritePtr = nullptr; // Point within m_vtxBuffer after each add command (to avoid using the vector operators too much)
	DrawIdx* m_idxWritePtr = nullptr; // Index within m_idxBuffer after each add command (to avoid using the vector operators too much)
	std::vector<unsigned int> m_textureIdStack;
	std::vector<panda::types::Point> m_path;
	std::vector<panda::types::Rect> m_clipRectStack;
};
