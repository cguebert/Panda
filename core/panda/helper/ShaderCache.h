#ifndef HELPER_SHADERCACHE_H
#define HELPER_SHADERCACHE_H

#include <panda/core.h>

#include <unordered_map>
#include <memory>

#include <QOpenGLShader>

namespace panda
{

namespace helper
{

class PANDA_CORE_API ShaderCache : public QObject
{
public:
	static ShaderCache* getInstance();

	void clear(); // Remove all shaders
	void resetUsedFlag();	// Prepare the flags at the start of a render
	void clearUnused();		// Remove shaders not used during the last render

	QOpenGLShader* getShader(QOpenGLShader::ShaderType type, const std::string& sourceCode, std::size_t hash = 0);

private:
	QOpenGLShader* addShader(QOpenGLShader::ShaderType type, const std::string& sourceCode, std::size_t hash);

	struct CacheItem
	{
		CacheItem(QOpenGLShader::ShaderType type, uint hash)
			: m_sourceHash(hash), m_type(type), m_used(true) {}

		std::size_t m_sourceHash;
		QOpenGLShader::ShaderType m_type;
		std::shared_ptr<QOpenGLShader> m_shader;
		bool m_used;
	};

	typedef std::unordered_multimap<std::size_t, CacheItem> ShaderTableHash;
	ShaderTableHash m_cache;
};

} // namespace helper

} // namespace panda

#endif // HELPER_SHADERCACHE_H
