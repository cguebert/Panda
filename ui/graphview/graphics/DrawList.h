#pragma once

#include <ui/graphview/graphics/DrawPath.h>

#include <panda/types/Rect.h>

#include <string>
#include <vector>

class Font;
class QColor;

using pPoint = panda::types::Point;
using pRect = panda::types::Rect;

class DrawList
{
public:
	using DrawIdx = unsigned int;

	struct DrawCmd
	{
		int elemCount = 0; // Number of indices (multiple of 3) to be rendered as triangles.
		unsigned int textureId = 0; // User-provided texture ID
		panda::types::Rect clipRect; // Clipping rectangle (x1, y1, x2, y2)
	};

	struct DrawVert
	{
		panda::types::Point pos, uv;
		unsigned int col;
	};

	enum Align
	{
		Align_Left		= 1 << 0,
		Align_HCenter	= 1 << 1,
		Align_Right		= 1 << 2,

		Align_Top		= 1 << 3,
		Align_VCenter	= 1 << 4,
		Align_Bottom	= 1 << 5,

		Align_Default  = Align_Left | Align_Top,
		Align_Center  = Align_HCenter | Align_VCenter
	};
	using TextAlign = int;

	DrawList();
	void clear();

	static unsigned int convert(const QColor& col);
	static inline unsigned int convert(unsigned int col)
	{ return (col & 0xFF00FF00) | ((col & 0xFF) << 16) | ((col & 0xFF0000) >> 16); }
	static inline unsigned int setAlpha(unsigned int col, unsigned char alpha)
	{ return (alpha << 24) | (col & 0x00FFFFFF); }

	void addDrawCmd(); // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible
	void merge(DrawList& list, bool ignoreClip = true); // Add the list to this one, merging it in the last draw command if possible

	void pushTextureID(unsigned int texture_id);
	void popTextureID();

	void pushClipRect(const pRect& clip_rect);
	void popClipRect();

	// Primitives
	void addLine(const pPoint& a, const pPoint& b, unsigned int col, float thickness = 1.0f);
	void addRect(const pPoint& a, const pPoint& b, unsigned int col, float thickness = 1.0f, float rounding = 0.0f, int rounding_corners = 0x0F);        // a: upper-left, b: lower-right
	void addRectFilled(const pPoint& a, const pPoint& b, unsigned int col, float rounding = 0.0f, int rounding_corners = 0x0F);  // a: upper-left, b: lower-right
	void addRectFilledMultiColor(const pPoint& a, const pPoint& b, unsigned int col_upr_left, unsigned int col_upr_right, unsigned int col_bot_right, unsigned int col_bot_left);
	void addTriangle(const pPoint& a, const pPoint& b, const pPoint& c, unsigned int col, float thickness);
	void addTriangleFilled(const pPoint& a, const pPoint& b, const pPoint& c, unsigned int col);
	void addCircle(const pPoint& centre, float radius, unsigned int col, int num_segments = 12, float thickness = 1.0f);
	void addCircleFilled(const pPoint& centre, float radius, unsigned int col, int num_segments = 12);
	void addBezierCurve(const pPoint& pos0, const pPoint& cp0, const pPoint& cp1, const pPoint& pos1, unsigned int col, float thickness, int num_segments = 0);
	void addText(const pPoint& pos, const std::string& text, unsigned int col);
	void addText(const pRect& rect, const std::string& text, unsigned int col, TextAlign align = Align_Default, float scale = 1.0f, bool wrap = true, bool fit = false); // fit: if true, will decrease the scale so that the text can fit in the rectangle
	void addText(const Font& font, const pPoint& pos, const std::string& text, unsigned int col, float font_scale = 1.0f, float wrap_width = 0.0f, const panda::types::Rect* cpu_fine_clip_rect = NULL);
	void addImage(unsigned int texture_id, const pPoint& a, const pPoint& b, const pPoint& uv0 = panda::types::Point(0,0), const pPoint& uv1 = panda::types::Point(1,1), unsigned int col = 0xFFFFFFFF);
	void addPolyline(const DrawPath& path, unsigned int col, bool close = true, float thickness = 1.0f, bool anti_aliased = true);
	void addConvexPolyFilled(const DrawPath& path, unsigned int col, bool anti_aliased = true);
	void addMesh(const DrawMesh& mesh, unsigned int col);

	pPoint calcTextSize(float scale, const std::string& text, float wrap_width = 0.0f, bool cutWords = false);

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
	void primRect(const pPoint& a, const pPoint& b, unsigned int col);      // Axis aligned rectangle (composed of two triangles)
	void primRectUV(const pPoint& a, const pPoint& b, const pPoint& uv_a, const pPoint& uv_b, unsigned int col);
	inline void primVtx(const pPoint& pos, const pPoint& uv, unsigned int col)
	{ primWriteIdx(m_vtxCurrentIdx); primWriteVtx(pos, uv, col); }
	inline void primWriteVtx(const pPoint& pos, const pPoint& uv, unsigned int col)
	{ m_vtxWritePtr->pos = pos; m_vtxWritePtr->uv = uv; m_vtxWritePtr->col = col; m_vtxWritePtr++; m_vtxCurrentIdx++; }
	inline void primWriteIdx(DrawIdx idx) { *m_idxWritePtr = idx; m_idxWritePtr++; }

	inline void pathFill(unsigned int col) { addConvexPolyFilled(m_path, col, true); m_path.clear(); }
	inline void pathStroke(unsigned int col, bool close, float thickness = 1.0f) { addPolyline(m_path, col, close, thickness, true); m_path.clear(); }

	void updateTextureID();
	void updateClipRect();
	
	std::vector<DrawCmd> m_cmdBuffer; // Commands. Typically 1 command = 1 gpu draw call.
	std::vector<DrawIdx> m_idxBuffer; // Index buffer. Each command consume DrawCmd::ElemCount of those
	std::vector<DrawVert> m_vtxBuffer; // Vertex buffer.

	DrawIdx m_vtxCurrentIdx = 0; // VtxBuffer.size()
	DrawVert* m_vtxWritePtr = nullptr; // Point within m_vtxBuffer after each add command (to avoid using the vector operators too much)
	DrawIdx* m_idxWritePtr = nullptr; // Index within m_idxBuffer after each add command (to avoid using the vector operators too much)
	std::vector<unsigned int> m_textureIdStack;
	std::vector<panda::types::Rect> m_clipRectStack;

	DrawPath m_path;
};
