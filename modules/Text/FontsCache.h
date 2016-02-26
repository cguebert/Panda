#pragma once

#include <freetype-gl/freetype-gl.h>

#include "TextureFont.h"

#include <memory>
#include <unordered_map>

namespace panda
{

class FontsCache
{
public:
	static FontsCache& instance();

	std::shared_ptr<TextureFont> getFont(const std::string& fontString);

private:
	std::unordered_map<std::string, std::weak_ptr<TextureFont>> m_textureFontsCache;
};

} // namespace panda
