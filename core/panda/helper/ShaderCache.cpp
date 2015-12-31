#include <panda/helper/ShaderCache.h>

#include <functional>
#include <string>

namespace panda
{

namespace helper
{

ShaderCache* ShaderCache::getInstance()
{
	static ShaderCache instance;
	return &instance;
}

void ShaderCache::clear()
{
	m_cache.clear();
}

void ShaderCache::resetUsedFlag()
{
	for(auto& item : m_cache)
		item.second.m_used = false;
}

void ShaderCache::clearUnused()
{
	ShaderTableHash::iterator it = m_cache.begin(), itEnd = m_cache.end();
	while(it!=itEnd)
	{
		if(!it->second.m_used)
			it = m_cache.erase(it);
		else
			++it;
	}
}

graphics::ShaderId::SPtr ShaderCache::getShader(graphics::ShaderType type, const std::string& sourceCode, std::size_t hash)
{
	if(!hash)
		hash = std::hash<std::string>()(sourceCode);
	ShaderTableHash::iterator it = m_cache.find(hash), itEnd = m_cache.end();

	if(it == itEnd)
		return addShader(type, sourceCode, hash);
	else
	{
		for (; it != itEnd && it->first == hash; ++it)
		{
			CacheItem& item = it->second;
			if(item.m_type == type)
			{
				item.m_used = true;
				return item.m_shader;
			}
		} 
		return addShader(type, sourceCode, hash);
	}
}

graphics::ShaderId::SPtr ShaderCache::addShader(graphics::ShaderType type, const std::string& sourceCode, std::size_t hash)
{
	auto id = graphics::ShaderProgram::compileShader(type, sourceCode);
	if (!id)
		return nullptr;

	CacheItem item(type, id, hash);
	m_cache.emplace(hash, item);

	return id;
}

} // namespace helper

} // namespace panda
