#include <panda/helper/ShaderCache.h>

#include <QOpenGLShaderProgram>

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
	for(auto item : m_cache)
		item.m_used = false;
}

void ShaderCache::clearUnused()
{
	ShaderTableHash::iterator it = m_cache.begin(), itEnd = m_cache.end();
	while(it!=itEnd)
	{
		if(!it.value().m_used)
			it = m_cache.erase(it);
		else
			++it;
	}
}

QOpenGLShader* ShaderCache::getShader(QOpenGLShader::ShaderType type, QByteArray sourceCode, unsigned int hash)
{
	if(!hash)
		hash = qHash(sourceCode);
	ShaderTableHash::iterator it = m_cache.find(hash), itEnd = m_cache.end();

	if(it == itEnd)
		return addShader(type, sourceCode, hash);
	else
	{
		do {
			CacheItem& item = it.value();
			if(item.m_type == type)
			{
				item.m_used = true;
				return item.m_shader.data();
			}
			++it;
		} while(it != itEnd && it.key() == hash);
		return addShader(type, sourceCode, hash);
	}
}

QOpenGLShader* ShaderCache::addShader(QOpenGLShader::ShaderType type, QByteArray sourceCode, unsigned int hash)
{
	CacheItem item(type, hash);

	item.m_shader = QSharedPointer<QOpenGLShader>(new QOpenGLShader(type, this));
	item.m_shader->compileSourceCode(sourceCode);

	m_cache.insert(hash, item);

	return item.m_shader.data();
}

} // namespace helper

} // namespace panda
