#include "FontsCache.h"

#include <panda/helper/Font.h>
#include <panda/helper/system/FileRepository.h>

namespace panda
{

FontsCache& FontsCache::instance()
{
	static FontsCache instance;
	return instance;
}

std::shared_ptr<TextureFont> FontsCache::getFont(const std::string& fontString)
{
	auto it = m_textureFontsCache.find(fontString);
	if (it != m_textureFontsCache.end())
	{
		if (!it->second.expired())
			return it->second.lock();
	}

	helper::Font font(fontString);
	if (font.path.empty())
		font = helper::Font();

	auto fontPath = helper::system::DataRepository.findFile(font.path);
	if (fontPath.empty())
		return nullptr;

	const char* cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
		"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
		"`abcdefghijklmnopqrstuvwxyz{|}~";

	for (int dim = 512; dim <= 4096; dim *= 2)
	{
		auto atlas = ftgl::texture_atlas_new(dim, dim, 1);
		auto texFont = ftgl::texture_font_new_from_file(atlas, static_cast<float>(font.pointSize), fontPath.c_str());

		auto missed = ftgl::texture_font_load_glyphs(texFont, cache);
		if (!missed)
		{
			auto ptr = std::make_shared<TextureFont>(atlas, texFont);
			m_textureFontsCache.emplace(fontString, ptr);
			return ptr;
		}

		ftgl::texture_font_delete(texFont);
		ftgl::texture_atlas_delete(atlas);
	}

	return nullptr;
}

} // namespace panda
