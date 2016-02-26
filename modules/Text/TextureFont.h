#pragma once

#include <freetype-gl/freetype-gl.h>

namespace panda
{

class TextureFont
{
public:
	TextureFont(ftgl::texture_atlas_t* atlas, ftgl::texture_font_t* textureFont);
	~TextureFont();

	ftgl::texture_font_t* fontPtr();
	unsigned int texId();

private:
	ftgl::texture_atlas_t* m_atlas = nullptr;
	ftgl::texture_font_t* m_texFont = nullptr;
};

inline ftgl::texture_font_t* TextureFont::fontPtr()
{ return m_texFont; }

inline unsigned int TextureFont::texId()
{ return m_atlas->id; }

} // namespace panda
