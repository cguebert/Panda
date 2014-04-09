#ifndef TYPES_SHADER_H
#define TYPES_SHADER_H

#include <panda/helper/system/Config.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/DataTraits.h>

#include <QFlags>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QOpenGLShader>

class QOpenGLShaderProgram;

namespace panda
{

namespace types
{

class BaseShaderValue
{
public:
	virtual void apply(QOpenGLShaderProgram& program) const = 0; // Add the value to the shader program
	virtual void cleanup() const = 0;	// Disable the array from the shader after rendering
	virtual QString getName() const = 0;
	virtual AbstractDataTrait* dataTrait() const = 0;
	virtual const void* getValue() const = 0;
	virtual void* getValue() = 0;
};

template<class T>
class ShaderValue : public BaseShaderValue
{
public:
	ShaderValue(QString name, const T& val) : m_name(name), m_value(val) { }
	virtual void apply(QOpenGLShaderProgram& program) const;
	virtual void cleanup() const {}
	virtual QString getName() const
	{ return m_name; }
	virtual AbstractDataTrait* dataTrait() const
	{ return DataTraitsList::getTraitOf<T>(); }
	virtual const void* getValue() const
	{ return &m_value; }
	virtual void* getValue()
	{ return &m_value; }

protected:
	QString m_name;
	T m_value;
};

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

	template<class T>
	void setUniform(QString name, const T& value)
	{
		m_shaderValues.push_back(QSharedPointer<BaseShaderValue>(new ShaderValue<T>(name, value)));
	}

	typedef QVector< QSharedPointer< BaseShaderValue > > ValuesVector;
	const ValuesVector& getValues() const;

protected:
	typedef QMap<QOpenGLShader::ShaderType, ShaderSource> SourcesMap;
	SourcesMap m_sourcesMap;

	ValuesVector m_shaderValues;
};


} // namespace types

} // namespace panda

#endif // TYPES_SHADER_H
