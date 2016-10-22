#pragma once

#include <ui/graphview/graphics/Font.h>

#include <memory>
#include <vector>
#include <string>
#include <utility>

namespace graphview
{

namespace graphics
{

// Load and rasterize multiple TTF fonts into a same texture.
// Sharing a texture for multiple fonts allows us to reduce the number of draw calls during rendering.
// We also add custom graphic data into the texture that serves for ImGui.
//  1. (Optional) Call addFont*** functions. If you don't call any, the default font will be loaded for you.
//  2. Call getTexDataAsAlpha8() or getTexDataAsRGBA32() to build and retrieve pixels data.
//  3. Upload the pixels data into a texture within your graphics system.
//  4. Call setTexID(my_tex_id); and pass the pointer/identifier to your texture. This value will be passed back to you during rendering to identify the texture.
//  5. Call clearTexData() to free textures memory on the heap.
class FontAtlas
{
public:
	~FontAtlas();

	using FontPtr = std::shared_ptr<Font>;

	FontPtr addFont(const FontConfig& font_cfg);
	FontPtr addFontDefault(const FontConfig& font_cfg);
	FontPtr addFontFromFileTTF(const char* filename, float size_pixels, const FontConfig& font_cfg, const GlyphRangeList& glyph_ranges);
	FontPtr addFontFromMemoryTTF(const MemBuffer& ttf_data, float size_pixels, const FontConfig& font_cfg, const GlyphRangeList& glyph_ranges);                                        // Transfer ownership of 'ttf_data' to FontAtlas, will be deleted after Build()
	FontPtr addFontFromMemoryCompressedTTF(const MemBuffer& compressed_ttf_data, float size_pixels, const FontConfig& font_cfg, const GlyphRangeList& glyph_ranges);  // 'compressed_ttf_data' still owned by caller. Compress with binary_to_compressed_c.cpp
	FontPtr addFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const FontConfig& font_cfg, const GlyphRangeList& glyph_ranges);              // 'compressed_ttf_data_base85' still owned by caller. Compress with binary_to_compressed_c.cpp with -base85 paramaeter
	void clearTexData();	// Clear the CPU-side texture data. Saves RAM once the texture has been copied to graphics memory.
	void clearInputData();	// Clear the input TTF data (inc sizes, glyph ranges)
	void clearFonts();		// Clear the ImGui-side font data (glyphs storage, UV coordinates)
	void clear();			// Clear all

	// Retrieve texture data
	// User is in charge of copying the pixels into graphics memory, then call SetTextureUserID()
	// After loading the texture into your graphic system, store your texture handle in 'TexID' (ignore if you aren't using multiple fonts nor images)
	// RGBA32 format is provided for convenience and high compatibility, but note that all RGB pixels are white, so 75% of the memory is wasted.
	// Pitch = Width * BytesPerPixels
	void getTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL);  // 1 byte per-pixel
	void getTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel = NULL);  // 4 bytes-per-pixel
	
	unsigned int texID() const { return m_texID; }
	void setTexID(unsigned int id)  { m_texID = id; }

	GlyphRangeList getGlyphRangesDefault();    // Basic Latin, Extended Latin

	const std::vector<FontPtr>& fonts() const { return m_fonts; }

private:
	unsigned int m_texID;				// User data to refer to the texture once it has been uploaded to user's graphic systems. It ia passed back to you during rendering.
	std::vector<unsigned char> m_texPixelsAlpha8;	// 1 component per pixel, each component is unsigned 8-bit. Total size = TexWidth * TexHeight
	std::vector<unsigned int> m_texPixelsRGBA32;	// 4 component per pixel, each component is unsigned 8-bit. Total size = TexWidth * TexHeight * 4
	int m_texWidth;						// Texture width calculated during build().
	int m_texHeight;					// Texture height calculated during build().
	int m_texDesiredWidth;				// Texture width desired by user before build(). Must be a power-of-two. If have many glyphs your graphics API have texture size restrictions you may want to increase texture width to decrease height.
	pPoint m_texUvWhitePixel;			// Texture coordinates to a white pixel
	std::vector<FontPtr> m_fonts;			// Hold all the fonts returned by addFont*. Fonts[0] is the default font, use ImGui::PushFont()/PopFont() to change the current font.
	
	std::vector<FontConfig> m_configData;         // Internal data
	bool build();
	void renderCustomTexData(int pass, void* rects);
};

} // namespace graphics

} // namespace graphview
