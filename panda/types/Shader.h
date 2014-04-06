#ifndef TYPES_SHADER_H
#define TYPES_SHADER_H

#include <QFlags>
#include <QString>
#include <QMap>
#include <QOpenGLShader>

class QOpenGLShaderProgram;

namespace panda
{

namespace types
{

class Shader
{
public:
	Shader();

	void clear(); /// Remove all sources & all data
	void addSource(QOpenGLShader::ShaderType type, QString sourceCode);
	void removeSource(QOpenGLShader::ShaderType type);

	void apply(QOpenGLShaderProgram& program) const;

	struct ShaderSource
	{
		QOpenGLShader::ShaderType type;
		QByteArray sourceCode;
		unsigned int hash;
	};

	const QList<ShaderSource> getSources() const;

protected:
	typedef QMap<QOpenGLShader::ShaderType, ShaderSource> SourcesMap;
	SourcesMap m_sourcesMap;
};


} // namespace types

} // namespace panda

#endif // TYPES_SHADER_H
