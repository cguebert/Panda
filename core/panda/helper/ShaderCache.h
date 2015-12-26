#ifndef HELPER_SHADERCACHE_H
#define HELPER_SHADERCACHE_H

#include <panda/core.h>

#include <unordered_map>

#include <QOpenGLShader>
#include <QSharedPointer>

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

	QOpenGLShader* getShader(QOpenGLShader::ShaderType type, QByteArray sourceCode, unsigned int hash = 0);

private:
	QOpenGLShader* addShader(QOpenGLShader::ShaderType type, QByteArray sourceCode, unsigned int hash);

	struct CacheItem
	{
		CacheItem(QOpenGLShader::ShaderType type, uint hash)
			: m_sourceHash(hash), m_type(type), m_used(true) {}

		unsigned int m_sourceHash;
		QOpenGLShader::ShaderType m_type;
		QSharedPointer<QOpenGLShader> m_shader;
		bool m_used;
	};

	typedef std::unordered_multimap<quint64, CacheItem> ShaderTableHash;
	ShaderTableHash m_cache;
};

} // namespace helper

} // namespace panda

#endif // HELPER_SHADERCACHE_H
