#ifndef HELPER_SHADERCACHE_H
#define HELPER_SHADERCACHE_H

#include <panda/core.h>

#include <panda/graphics/ShaderProgram.h>

#include <unordered_map>

namespace panda
{

namespace helper
{

class PANDA_CORE_API ShaderCache
{
public:
	static ShaderCache* getInstance();

	void clear(); // Remove all shaders
	void resetUsedFlag();	// Prepare the flags at the start of a render
	void clearUnused();		// Remove shaders not used during the last render

	graphics::ShaderId::SPtr getShader(graphics::ShaderType type,std::size_t hash);
	graphics::ShaderId::SPtr getShader(graphics::ShaderType type, const std::string& sourceCode, std::size_t hash = 0);

	using ShaderPair = std::pair<graphics::ShaderType, std::size_t>;
	using ShadersList = std::vector<ShaderPair>;
	graphics::ShaderProgram getShaderProgram(const ShadersList& shaders);

private:
	graphics::ShaderId::SPtr addShader(graphics::ShaderType type, const std::string& sourceCode, std::size_t hash);

	struct ShaderCacheItem
	{
		ShaderCacheItem(graphics::ShaderType type, graphics::ShaderId::SPtr id, size_t hash)
			: m_type(type), m_shader(id), m_sourceHash(hash), m_used(true) {}

		graphics::ShaderType m_type;
		graphics::ShaderId::SPtr m_shader;
		std::size_t m_sourceHash;
		bool m_used;
	};

	using ShadersTableHash = std::unordered_multimap<std::size_t, ShaderCacheItem>;
	ShadersTableHash m_shadersCache;

	struct ProgramCacheItem
	{
		ProgramCacheItem(const ShadersList& shaders, graphics::ShaderProgramId::SPtr program)
			: m_shaders(shaders), m_program(program) {}

		ShadersList m_shaders;
		graphics::ShaderProgramId::SPtr m_program;
		bool m_used;
	};

	using ProgramsList = std::vector<ProgramCacheItem>;
	ProgramsList m_programsCache;
};

} // namespace helper

} // namespace panda

#endif // HELPER_SHADERCACHE_H
