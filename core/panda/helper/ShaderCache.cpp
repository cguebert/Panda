#include <panda/helper/ShaderCache.h>
#include <panda/helper/system/FileRepository.h>

#include <algorithm>
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
	m_shadersCache.clear();
}

void ShaderCache::resetUsedFlag()
{
	for(auto& item : m_shadersCache)
		item.second.m_used = false;

	for(auto& item : m_programsCache)
		item.m_used = false;
}

void ShaderCache::clearUnused()
{
	auto it = m_shadersCache.begin(), itEnd = m_shadersCache.end();
	while(it != itEnd)
	{
		if(!it->second.m_used)
			it = m_shadersCache.erase(it);
		else
			++it;
	}

	auto last = std::remove_if(m_programsCache.begin(), m_programsCache.end(), [](const ProgramCacheItem& item) {
		return !item.m_used;
	});
	m_programsCache.erase(last, m_programsCache.end());
}

graphics::ShaderId::SPtr ShaderCache::getShader(graphics::ShaderType type, std::size_t hash)
{
	auto it = m_shadersCache.find(hash), itEnd = m_shadersCache.end();

	if(it != itEnd)
	{
		for (; it != itEnd && it->first == hash; ++it)
		{
			auto& item = it->second;
			if(item.m_type == type)
			{
				item.m_used = true;
				return item.m_shader;
			}
		} 
	}

	return nullptr;
}

graphics::ShaderId::SPtr ShaderCache::getShaderFromSource(graphics::ShaderType type, const std::string& sourceCode, std::size_t hash)
{
	if(!hash)
		hash = std::hash<std::string>()(sourceCode);

	auto shader = getShader(type, hash);
	if (shader)
		return shader;
	
	return addShader(type, sourceCode, hash);
}

graphics::ShaderId::SPtr ShaderCache::getShaderFromFile(graphics::ShaderType type, const std::string& filePath)
{
	auto& repository = panda::helper::system::DataRepository;
	auto realPath = repository.findFile(filePath);
	if(realPath.empty())
		return nullptr;

	auto sourceCode = repository.loadFile(realPath);
	auto hash = std::hash<std::string>()(sourceCode);

	auto shader = getShader(type, hash);
	if (shader)
		return shader;
	
	return addShader(type, sourceCode, hash);
}

graphics::ShaderProgram ShaderCache::getShaderProgram(const ShadersList& shaders)
{
	auto it = std::find_if(m_programsCache.begin(), m_programsCache.end(), [&shaders](const ProgramCacheItem& item) {
		return item.m_shaders == shaders;
	});

	if (it != m_programsCache.end())
	{
		it->m_used = true;
		return graphics::ShaderProgram(it->m_program);
	}

	graphics::ShaderProgram program;
	for (const auto& shader : shaders)
	{
		auto shaderId = getShader(shader.first, shader.second);
		program.addShader(shaderId);
	}

	program.link();
	ProgramCacheItem item(shaders, program.getProgramId());
	m_programsCache.push_back(std::move(item));

	return program;
}

graphics::ShaderProgram ShaderCache::getShaderProgramFromSource(const ShadersSourceList& shaders)
{
	helper::ShaderCache::ShadersList shadersList;
	for (const auto& s : shaders)
	{
		auto hash = std::hash<std::string>()(s.second);
		if(!getShaderFromSource(s.first, s.second, hash))
			return graphics::ShaderProgram();
		shadersList.emplace_back(s.first, hash);
	}

	return getShaderProgram(shadersList);
}

graphics::ShaderProgram ShaderCache::getShaderProgramFromFile(const ShadersFileList& shaders)
{
	auto& repository = panda::helper::system::DataRepository;
	helper::ShaderCache::ShadersList shadersList;
	for (const auto& s : shaders)
	{
		auto path = repository.findFile(s.second);
		if(path.empty())
			return graphics::ShaderProgram();

		auto sourceCode = repository.loadFile(path);
		auto hash = std::hash<std::string>()(sourceCode);
		if(!getShaderFromSource(s.first, sourceCode, hash))
			return graphics::ShaderProgram();
		shadersList.emplace_back(s.first, hash);
	}

	return getShaderProgram(shadersList);
}

graphics::ShaderId::SPtr ShaderCache::addShader(graphics::ShaderType type, const std::string& sourceCode, std::size_t hash)
{
	auto id = graphics::ShaderProgram::compileShader(type, sourceCode);
	if (!id)
		return nullptr;

	ShaderCacheItem item(type, id, hash);
	m_shadersCache.emplace(hash, item);

	return id;
}

} // namespace helper

} // namespace panda
