#include "TextureFont.h"

#include <cassert>

namespace panda
{

TextureFont::TextureFont(ftgl::texture_atlas_t* atlas, ftgl::texture_font_t* textureFont)
	: m_atlas(atlas)
	, m_texFont(textureFont)
{
	assert(m_atlas);
	assert(m_texFont);
}

TextureFont::~TextureFont()
{
	if(m_texFont)
		ftgl::texture_font_delete(m_texFont);

	if (m_atlas)
		ftgl::texture_atlas_delete(m_atlas);
}

} // namespace panda
