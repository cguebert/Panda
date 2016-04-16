#include <ui/graphview/graphics/FontAtlas.h>
#include <ui/graphview/graphics/DefaultFont.h>

#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include <ui/graphview/graphics/stb_rect_pack.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <ui/graphview/graphics/stb_truetype.h>

#include <algorithm>
#include <cassert>
#include <fstream>

namespace
{
	static inline int UpperPowerOfTwo(int v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

	MemBuffer LoadFileToMemory(const char* filename)
	{
		MemBuffer contents;
		std::ifstream in(filename, std::ios_base::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			contents.resize((size_t)in.tellg());
			in.seekg(0, std::ios::beg);
			in.read((char*)&contents[0], contents.size());
			in.close();
		}
		return contents;
	}
}

FontAtlas::~FontAtlas()
{
	clear();
}

void FontAtlas::clearInputData()
{
	for (size_t i = 0; i < m_configData.size(); i++)
		if (m_configData[i].FontDataOwnedByAtlas)
		{
			m_configData[i].FontData.clear();
		}

	// When clearing this we lose access to the font name and other information used to build the font.
	for (size_t i = 0; i < m_fonts.size(); i++)
		if (m_fonts[i]->m_configData >= m_configData.data() && m_fonts[i]->m_configData < m_configData.data() + m_configData.size())
		{
			m_fonts[i]->m_configData = NULL;
			m_fonts[i]->m_configDataCount = 0;
		}
	m_configData.clear();
}

void FontAtlas::clearTexData()
{
	m_texPixelsAlpha8.clear();
	m_texPixelsAlpha8.shrink_to_fit();
	m_texPixelsRGBA32.clear();
	m_texPixelsRGBA32.shrink_to_fit();
}

void FontAtlas::clearFonts()
{
	m_fonts.clear();
}

void FontAtlas::clear()
{
	clearInputData();
	clearTexData();
	clearFonts();
}

void FontAtlas::getTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
	// build atlas on demand
	if (m_texPixelsAlpha8.empty())
	{
		if (m_configData.empty())
		{
			FontConfig cfg;
			cfg.OversampleH = cfg.OversampleV = 1;
			cfg.PixelSnapH = true;
			cfg.Name = "<default>";
			addFontDefault(cfg);
		}
		build();
	}

	if (out_pixels) *out_pixels = m_texPixelsAlpha8.data();
	if (out_width) *out_width = m_texWidth;
	if (out_height) *out_height = m_texHeight;
	if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void FontAtlas::getTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
	// Convert to RGBA32 format on demand
	// Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
	if (m_texPixelsRGBA32.empty())
	{
		getTexDataAsAlpha8(NULL, NULL, NULL);
		m_texPixelsRGBA32.resize(m_texWidth * m_texHeight);
		for (int i = 0, n = m_texWidth * m_texHeight; i < n; ++i)
			m_texPixelsRGBA32[i] = (static_cast<unsigned int>(m_texPixelsAlpha8[i]) << 24) | 0x00FFFFFF;
	}

	*out_pixels = (unsigned char*)m_texPixelsRGBA32.data();
	if (out_width) *out_width = m_texWidth;
	if (out_height) *out_height = m_texHeight;
	if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

FontAtlas::FontPtr FontAtlas::addFont(const FontConfig& font_cfg)
{
	assert(!font_cfg.FontData.empty());
	assert(font_cfg.SizePixels > 0.0f);

	// Create new font
	if (!font_cfg.MergeMode)
		m_fonts.emplace_back(std::make_shared<Font>());

	m_configData.push_back(font_cfg);
	FontConfig& new_font_cfg = m_configData.back();
	new_font_cfg.DstFont = m_fonts.back();
	if (!new_font_cfg.FontDataOwnedByAtlas)
	{
		new_font_cfg.FontDataOwnedByAtlas = true;
		new_font_cfg.FontData = font_cfg.FontData;
	}

	// Invalidate texture
	clearTexData();
	return m_fonts.back();
}

// Default font TTF is compressed with stb_compress then base85 encoded (see extra_fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
	while (*src)
	{
		unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
		dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianess.
		src += 5;
		dst += 4;
	}
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
FontAtlas::FontPtr FontAtlas::addFontDefault(const FontConfig& font_cfg)
{
	const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
	return addFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, 13.0f, font_cfg, getGlyphRangesDefault());
}

FontAtlas::FontPtr FontAtlas::addFontFromFileTTF(const char* filename, float size_pixels, const FontConfig& font_cfg_template, const GlyphRangeList& glyph_ranges)
{
	MemBuffer data = LoadFileToMemory(filename);
	if (data.empty())
	{
		assert(0); // Could not load file.
		return NULL;
	}
	FontConfig font_cfg = font_cfg_template;
	if (font_cfg.Name[0] == '\0')
	{
		// Store a short copy of filename into into the font name for convenience
		const char* p;
		for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
		font_cfg.Name = p;
	}
	return addFontFromMemoryTTF(data, size_pixels, font_cfg, glyph_ranges);
}

// NBM Transfer ownership of 'ttf_data' to FontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after build().
FontAtlas::FontPtr FontAtlas::addFontFromMemoryTTF(const MemBuffer& ttf_data, float size_pixels, const FontConfig& font_cfg_template, const GlyphRangeList& glyph_ranges)
{
	FontConfig font_cfg = font_cfg_template;
	assert(font_cfg.FontData.empty());
	font_cfg.FontData = ttf_data;
	font_cfg.SizePixels = size_pixels;
	font_cfg.GlyphRanges = glyph_ranges;
	return addFont(font_cfg);
}

FontAtlas::FontPtr FontAtlas::addFontFromMemoryCompressedTTF(const MemBuffer& compressed_ttf, float size_pixels, const FontConfig& font_cfg_template, const GlyphRangeList& glyph_ranges)
{
	MemBuffer buf_decompressed(stb_decompress_length(compressed_ttf.data()));
	stb_decompress(buf_decompressed.data(), (unsigned char*)compressed_ttf.data(), compressed_ttf.size());

	FontConfig font_cfg = font_cfg_template;
	assert(font_cfg.FontData.empty());
	font_cfg.FontDataOwnedByAtlas = true;
	return addFontFromMemoryTTF(buf_decompressed, size_pixels, font_cfg, glyph_ranges);
}

FontAtlas::FontPtr FontAtlas::addFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const FontConfig& font_cfg, const GlyphRangeList& glyph_ranges)
{
	int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
	MemBuffer buf(compressed_ttf_size);
	Decode85((const unsigned char*)compressed_ttf_data_base85, buf.data());
	return addFontFromMemoryCompressedTTF(buf, size_pixels, font_cfg, glyph_ranges);
}

bool FontAtlas::build()
{
	assert(m_configData.size() > 0);

	m_texID = NULL;
	m_texWidth = m_texHeight = 0;
	m_texUvWhitePixel = { 0, 0 };
	clearTexData();

	struct FontTempBuildData
	{
		stbtt_fontinfo      FontInfo;
		stbrp_rect*         Rects;
		stbtt_pack_range*   Ranges;
		int                 RangesCount;
	};
	std::vector<FontTempBuildData> tmp_array(m_configData.size());

	// Initialize font information early (so we can error without any cleanup) + count glyphs
	int total_glyph_count = 0;
	int total_glyph_range_count = 0;
	for (size_t input_i = 0; input_i < m_configData.size(); ++input_i)
	{
		FontConfig& cfg = m_configData[input_i];
		FontTempBuildData& tmp = tmp_array[input_i];

		assert(cfg.DstFont && (!cfg.DstFont->isLoaded() || cfg.DstFont->m_containerAtlas == this));
		const int font_offset = stbtt_GetFontOffsetForIndex(cfg.FontData.data(), cfg.FontNo);
		assert(font_offset >= 0);
		if (!stbtt_InitFont(&tmp.FontInfo, cfg.FontData.data(), font_offset))
			return false;

		// Count glyphs
		if (cfg.GlyphRanges.empty())
			cfg.GlyphRanges = getGlyphRangesDefault();
		for (const GlyphRange& in_range : cfg.GlyphRanges)
		{
			total_glyph_count += (in_range.second - in_range.first) + 1;
			total_glyph_range_count++;
		}
	}

	// Start packing. We need a known width for the skyline algorithm. Using a cheap heuristic here to decide of width. User can override TexDesiredWidth if they wish.
	// After packing is done, width shouldn't matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
	m_texWidth = (m_texDesiredWidth > 0) ? m_texDesiredWidth : (total_glyph_count > 4000) ? 4096 : (total_glyph_count > 2000) ? 2048 : (total_glyph_count > 1000) ? 1024 : 512;
	m_texHeight = 0;
	const int max_tex_height = 1024*32;
	stbtt_pack_context spc;
	stbtt_PackBegin(&spc, NULL, m_texWidth, max_tex_height, 0, 1, NULL);

	// Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
	std::vector<stbrp_rect> extra_rects;
	renderCustomTexData(0, &extra_rects);
	stbtt_PackSetOversampling(&spc, 1, 1);
	stbrp_pack_rects((stbrp_context*)spc.pack_info, &extra_rects[0], extra_rects.size());
	for (size_t i = 0; i < extra_rects.size(); i++)
		if (extra_rects[i].was_packed)
			m_texHeight = std::max(m_texHeight, extra_rects[i].y + extra_rects[i].h);

	// Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
	int buf_packedchars_n = 0, buf_rects_n = 0, buf_ranges_n = 0;
	std::vector<stbtt_packedchar> packedchars;
	std::vector<stbrp_rect> rects;
	std::vector<stbtt_pack_range> ranges;
	packedchars.reserve(total_glyph_count);
	rects.reserve(total_glyph_count);
	ranges.reserve(total_glyph_range_count);
	auto buf_packedchars = packedchars.data();
	auto buf_rects = rects.data();
	auto buf_ranges = ranges.data();
	memset(buf_packedchars, 0, total_glyph_count * sizeof(stbtt_packedchar));
	memset(buf_rects, 0, total_glyph_count * sizeof(stbrp_rect));              // Unnecessary but let's clear this for the sake of sanity.
	memset(buf_ranges, 0, total_glyph_range_count * sizeof(stbtt_pack_range));

	// First font pass: pack all glyphs (no rendering at this point, we are working with rectangles in an infinitely tall texture at this point)
	for (size_t input_i = 0; input_i < m_configData.size(); input_i++)
	{
		FontConfig& cfg = m_configData[input_i];
		FontTempBuildData& tmp = tmp_array[input_i];

		// Setup ranges
		int glyph_count = 0;
		int glyph_ranges_count = cfg.GlyphRanges.size();
		for (const GlyphRange& in_range : cfg.GlyphRanges)
			glyph_count += (in_range.second - in_range.first) + 1;
		tmp.Ranges = buf_ranges + buf_ranges_n;
		tmp.RangesCount = glyph_ranges_count;
		buf_ranges_n += glyph_ranges_count;
		for (int i = 0; i < glyph_ranges_count; ++i)
		{
			const GlyphRange& in_range = cfg.GlyphRanges[i];
			stbtt_pack_range& range = tmp.Ranges[i];
			range.font_size = cfg.SizePixels;
			range.first_unicode_codepoint_in_range = in_range.first;
			range.num_chars = (in_range.second - in_range.first) + 1;
			range.chardata_for_range = buf_packedchars + buf_packedchars_n;
			buf_packedchars_n += range.num_chars;
		}

		// Pack
		tmp.Rects = buf_rects + buf_rects_n;
		buf_rects_n += glyph_count;
		stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
		int n = stbtt_PackFontRangesGatherRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
		stbrp_pack_rects((stbrp_context*)spc.pack_info, tmp.Rects, n);

		// Extend texture height
		for (int i = 0; i < n; i++)
			if (tmp.Rects[i].was_packed)
				m_texHeight = std::max(m_texHeight, tmp.Rects[i].y + tmp.Rects[i].h);
	}
	assert(buf_rects_n == total_glyph_count);
	assert(buf_packedchars_n == total_glyph_count);
	assert(buf_ranges_n == total_glyph_range_count);

	// Create texture
	m_texHeight = UpperPowerOfTwo(m_texHeight);
	m_texPixelsAlpha8.assign(m_texWidth * m_texHeight, 0);
	spc.pixels = m_texPixelsAlpha8.data();
	spc.height = m_texHeight;

	// Second pass: render characters
	for (size_t input_i = 0; input_i < m_configData.size(); input_i++)
	{
		FontConfig& cfg = m_configData[input_i];
		FontTempBuildData& tmp = tmp_array[input_i];
		stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
		stbtt_PackFontRangesRenderIntoRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
		tmp.Rects = NULL;
	}

	// End packing
	stbtt_PackEnd(&spc);

	// Third pass: setup Font and glyphs for runtime
	for (size_t input_i = 0; input_i < m_configData.size(); input_i++)
	{
		FontConfig& cfg = m_configData[input_i];
		FontTempBuildData& tmp = tmp_array[input_i];
		Font* dst_font = cfg.DstFont.get();

		float font_scale = stbtt_ScaleForPixelHeight(&tmp.FontInfo, cfg.SizePixels);
		int unscaled_ascent, unscaled_descent, unscaled_line_gap;
		stbtt_GetFontVMetrics(&tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

		float ascent = unscaled_ascent * font_scale;
		float descent = unscaled_descent * font_scale;
		if (!cfg.MergeMode)
		{
			dst_font->m_containerAtlas = this;
			dst_font->m_configData = &cfg;
			dst_font->m_configDataCount = 0;
			dst_font->m_fontSize = cfg.SizePixels;
			dst_font->m_ascent = ascent;
			dst_font->m_descent = descent;
			dst_font->m_glyphs.clear();
		}
		dst_font->m_configDataCount++;
		float off_y = (cfg.MergeMode && cfg.MergeGlyphCenterV) ? (ascent - dst_font->m_ascent) * 0.5f : 0.0f;

		dst_font->m_fallbackGlyph = NULL; // Always clear fallback so FindGlyph can return NULL. It will be set again in BuildLookupTable()
		for (int i = 0; i < tmp.RangesCount; i++)
		{
			stbtt_pack_range& range = tmp.Ranges[i];
			for (int char_idx = 0; char_idx < range.num_chars; char_idx += 1)
			{
				const stbtt_packedchar& pc = range.chardata_for_range[char_idx];
				if (!pc.x0 && !pc.x1 && !pc.y0 && !pc.y1)
					continue;

				const int codepoint = range.first_unicode_codepoint_in_range + char_idx;
				if (cfg.MergeMode && dst_font->findGlyph((unsigned short)codepoint))
					continue;

				stbtt_aligned_quad q;
				float dummy_x = 0.0f, dummy_y = 0.0f;
				stbtt_GetPackedQuad(range.chardata_for_range, m_texWidth, m_texHeight, char_idx, &dummy_x, &dummy_y, &q, 0);

				dst_font->m_glyphs.resize(dst_font->m_glyphs.size() + 1);
				Font::Glyph& glyph = dst_font->m_glyphs.back();
				glyph.Codepoint = (WChar)codepoint;
				glyph.X0 = q.x0; glyph.Y0 = q.y0; glyph.X1 = q.x1; glyph.Y1 = q.y1;
				glyph.U0 = q.s0; glyph.V0 = q.t0; glyph.U1 = q.s1; glyph.V1 = q.t1;
				glyph.Y0 += (float)(int)(dst_font->m_ascent + off_y + 0.5f);
				glyph.Y1 += (float)(int)(dst_font->m_ascent + off_y + 0.5f);
				glyph.XAdvance = (pc.xadvance + cfg.GlyphExtraSpacing.x);  // Bake spacing into XAdvance
				if (cfg.PixelSnapH)
					glyph.XAdvance = (float)(int)(glyph.XAdvance + 0.5f);
			}
		}
		cfg.DstFont->buildLookupTable();
	}

	// Render into our custom data block
	renderCustomTexData(1, &extra_rects);

	return true;
}

void FontAtlas::renderCustomTexData(int pass, void* p_rects)
{
	// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
	// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
	const int TEX_DATA_W = 90;
	const int TEX_DATA_H = 27;
	const char texture_data[TEX_DATA_W*TEX_DATA_H+1] =
	{
		"..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
		"..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
		"---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
		"X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
		"XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
		"X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
		"X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
		"X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
		"X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
		"X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
		"X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
		"X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
		"X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
		"X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
		"X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
		"X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
		"X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
		"X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
		"X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
		"XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
		"      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
		"       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
		"------------        -    X    -           X           -X.....................X-           "
		"                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
		"                                                      -  X..X           X..X  -           "
		"                                                      -   X.X           X.X   -           "
		"                                                      -    XX           XX    -           "
	};

	std::vector<stbrp_rect>& rects = *(std::vector<stbrp_rect>*)p_rects;
	if (pass == 0)
	{
		// Request rectangles
		stbrp_rect r;
		memset(&r, 0, sizeof(r));
		r.w = (TEX_DATA_W*2)+1;
		r.h = TEX_DATA_H+1;
		rects.push_back(r);
	}
	else if (pass == 1)
	{
		// Render/copy pixels
		const stbrp_rect& r = rects[0];
		for (int y = 0, n = 0; y < TEX_DATA_H; y++)
			for (int x = 0; x < TEX_DATA_W; x++, n++)
			{
				const int offset0 = (int)(r.x + x) + (int)(r.y + y) * m_texWidth;
				const int offset1 = offset0 + 1 + TEX_DATA_W;
				m_texPixelsAlpha8[offset0] = texture_data[n] == '.' ? 0xFF : 0x00;
				m_texPixelsAlpha8[offset1] = texture_data[n] == 'X' ? 0xFF : 0x00;
			}
		const pPoint tex_uv_scale(1.0f / m_texWidth, 1.0f / m_texHeight);
		m_texUvWhitePixel = pPoint((r.x + 0.5f) * tex_uv_scale.x, (r.y + 0.5f) * tex_uv_scale.y);
/*
		// Setup mouse cursors
		const pPoint cursor_datas[ImGuiMouseCursor_Count_][3] =
		{
			// Pos ........ Size ......... Offset ......
			{ pPoint(0,3),  pPoint(12,19), pPoint( 0, 0) }, // ImGuiMouseCursor_Arrow
			{ pPoint(13,0), pPoint(7,16),  pPoint( 4, 8) }, // ImGuiMouseCursor_TextInput
			{ pPoint(31,0), pPoint(23,23), pPoint(11,11) }, // ImGuiMouseCursor_Move
			{ pPoint(21,0), pPoint( 9,23), pPoint( 5,11) }, // ImGuiMouseCursor_ResizeNS
			{ pPoint(55,18),pPoint(23, 9), pPoint(11, 5) }, // ImGuiMouseCursor_ResizeEW
			{ pPoint(73,0), pPoint(17,17), pPoint( 9, 9) }, // ImGuiMouseCursor_ResizeNESW
			{ pPoint(55,0), pPoint(17,17), pPoint( 9, 9) }, // ImGuiMouseCursor_ResizeNWSE
		};

		for (int type = 0; type < ImGuiMouseCursor_Count_; type++)
		{
			ImGuiMouseCursorData& cursor_data = GImGui->MouseCursorData[type];
			pPoint pos = cursor_datas[type][0] + pPoint((float)r.x, (float)r.y);
			const pPoint size = cursor_datas[type][1];
			cursor_data.Type = type;
			cursor_data.size() = size;
			cursor_data.HotOffset = cursor_datas[type][2];
			cursor_data.TexUvMin[0] = (pos) * tex_uv_scale;
			cursor_data.TexUvMax[0] = (pos + size) * tex_uv_scale;
			pos.x += TEX_DATA_W+1;
			cursor_data.TexUvMin[1] = (pos) * tex_uv_scale;
			cursor_data.TexUvMax[1] = (pos + size) * tex_uv_scale;
		}
		*/
	}
}

// Retrieve list of range (2 int per range, values are inclusive)
GlyphRangeList FontAtlas::getGlyphRangesDefault()
{
	return { {0x0020, 0x00FF} };
}
