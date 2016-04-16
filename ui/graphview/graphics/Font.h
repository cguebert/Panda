#pragma once

#include <panda/types/Rect.h>

#include <memory>
#include <vector>
#include <string>
#include <utility>

using pPoint = panda::types::Point;
using pRect = panda::types::Rect;
using WChar = unsigned short;
using GlyphRange = std::pair<WChar, WChar>;
using GlyphRangeList = std::vector<GlyphRange>;
using MemBuffer = std::vector<unsigned char>;

class DrawList;
class Font;
class FontAtlas;

struct FontConfig
{
	MemBuffer FontData;							// TTF data
	bool	FontDataOwnedByAtlas = true;		// TTF data ownership taken by the container FontAtlas (will delete memory itself). Set to true
	int		FontNo = 0;							// Index of font within TTF file
	float	SizePixels = 0.f;					// Size in pixels for rasterizer
	int		OversampleH = 3, OversampleV = 1;	// Rasterize at higher quality for sub-pixel positioning. We don't use sub-pixel positions on the Y axis.
	bool	PixelSnapH = false;					// Align every character to pixel boundary (if enabled, set OversampleH/V to 1)
	pPoint	GlyphExtraSpacing = { 0, 0 };		// Extra spacing (in pixels) between glyphs
	GlyphRangeList GlyphRanges;					// List of Unicode range (2 value per range, values are inclusive, zero-terminated list)
	bool	MergeMode = false;					// Merge into previous Font, so you can combine multiple inputs font into one Font (e.g. ASCII font + icons + Japanese glyphs).
	bool	MergeGlyphCenterV = false;			// When merging (multiple FontInput for one Font), vertically center new glyphs instead of aligning their baseline

	// [Internal]
	std::string Name; // Name (strictly for debugging)
	std::shared_ptr<Font> DstFont;
};

// Font runtime data and rendering
// FontAtlas automatically loads a default embedded font for you when you call GetTexDataAsAlpha8() or GetTexDataAsRGBA32().
class Font
{
public:
	struct Glyph
	{
		WChar Codepoint;
		float XAdvance;
		float X0, Y0, X1, Y1;
		float U0, V0, U1, V1;     // Texture coordinates
	};

	 // Methods
	Font();
	~Font();
	void clear();
	void buildLookupTable();
	const Glyph* findGlyph(unsigned short c) const;
	void setFallbackChar(WChar c);
	float getCharAdvance(unsigned short c) const { return (c < (int)m_indexXAdvance.size()) ? m_indexXAdvance[c] : m_fallbackXAdvance; }

	bool isLoaded() const { return m_containerAtlas != NULL; }
	FontAtlas* atlas() const { return m_containerAtlas; }
	float fontSize() const { return m_fontSize; }

	// 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to disable.
	// 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given width. 0.0f to disable.
	pPoint calcTextSize(float scale, float wrap_width, const std::string& text, bool cutWords = true) const; // utf8
	const char* calcWordWrapPosition(float scale, const char* text, const char* text_end, float wrap_width, bool cutWords = true) const;
	void renderText(float scale, pPoint pos, unsigned int col, const pRect& clip_rect, const std::string& text, DrawList* draw_list, float wrap_width = 0.0f, bool cpu_fine_clip = false) const;

private:
	friend FontAtlas;

	// Members: Settings
	float m_fontSize = 0.f;				// Height of characters, set during loading (don't change after loading)
	pPoint m_displayOffset = { 0, 1 };	// Offset font rendering by xx pixels
	WChar m_fallbackChar = '?';			// Replacement glyph if one isn't found. Only set via SetFallbackChar()
	FontConfig* m_configData = nullptr; // Pointer within FontAtlas->ConfigData
	int m_configDataCount = 0;
	
	// Members: Runtime data
	float m_ascent = 0.f, m_descent = 0.f; // Ascent: distance from top to bottom of e.g. 'A' [0..FontSize]
	FontAtlas* m_containerAtlas = nullptr;
	std::vector<Glyph> m_glyphs;
	const Glyph* m_fallbackGlyph = nullptr;
	float m_fallbackXAdvance = 0.f;
	std::vector<float> m_indexXAdvance; // Sparse. Glyphs->XAdvance directly indexable (more cache-friendly that reading from Glyphs, for CalcTextSize functions which are often bottleneck in large UI)
	std::vector<int> m_indexLookup; // Sparse. Index glyphs by Unicode code-point.
};

