#ifndef TYPES_SHADER_H
#define TYPES_SHADER_H

#include <panda/core.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/DataTraits.h>

#include <QFlags>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QOpenGLShader>
#include <QDomElement>

#include <tuple>

class QOpenGLShaderProgram;

namespace panda
{

namespace types
{

class Shader;

class BaseShaderValue
{
public:
	virtual void apply(QOpenGLShaderProgram& program, const Shader& shader) const = 0; // Add the value to the shader program
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
	virtual void apply(QOpenGLShaderProgram& program, const Shader& shader) const;
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

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API ShaderValue<int>;
extern template class PANDA_CORE_API ShaderValue<PReal>;
extern template class PANDA_CORE_API ShaderValue<Color>;
extern template class PANDA_CORE_API ShaderValue<Point>;
extern template class PANDA_CORE_API ShaderValue<QVector<int>>;
extern template class PANDA_CORE_API ShaderValue<QVector<PReal>>;
extern template class PANDA_CORE_API ShaderValue<QVector<Color>>;
extern template class PANDA_CORE_API ShaderValue<QVector<Point>>;
#endif

//****************************************************************************//

typedef std::tuple<int, PReal, Color, Point,
	QVector<int>, QVector<PReal>, QVector<Color>, QVector<Point> > shaderValuesTypes;

class PANDA_CORE_API Shader
{
public:
	Shader();

	Shader& operator=(const Shader& shader);

	void clear(); /// Remove all sources & all data
	void setSource(QOpenGLShader::ShaderType type, const QString& sourceCode);
	void setSourceFromFile(QOpenGLShader::ShaderType type, const QString& fileName);
	void removeSource(QOpenGLShader::ShaderType type);

	bool apply(QOpenGLShaderProgram& program) const;

	struct ShaderSource
	{
		QOpenGLShader::ShaderType type;
		QByteArray sourceCode;
		unsigned int hash;

		bool operator==(const ShaderSource& s) const
		{ return hash == s.hash; }
	};

	const QList<ShaderSource> getSources() const;

	template<class T>
	void setUniform(QString name, const T& value)
	{
		m_shaderValues.push_back(QSharedPointer<BaseShaderValue>(new ShaderValue<T>(name, value)));
	}

	template<>
	void setUniform(QString name, const ImageWrapper& img)
	{
		m_customTextures.push_back(qMakePair(name, img.getTextureId()));
	}

	typedef QVector< QSharedPointer< BaseShaderValue > > ValuesVector;
	const ValuesVector& getValues() const;

	void loadValue(QString type, QDomElement &elem);
	void copyValue(QString type, QString name, const void* value);

	bool operator==(const Shader& s) const;
	bool operator!=(const Shader& s) const;

protected:
	typedef QMap<QOpenGLShader::ShaderType, ShaderSource> SourcesMap;
	SourcesMap m_sourcesMap;

	ValuesVector m_shaderValues;

	typedef void(Shader::*loadValueFuncPtr)(QDomElement&);
	QMap<QString, loadValueFuncPtr> m_loadValueFunctions;

	typedef void(Shader::*copyValueFuncPtr)(QString, const void*);
	QMap<QString, copyValueFuncPtr> m_copyValueFunctions;

	struct functionCreatorWrapper
	{
		Shader* object;
		functionCreatorWrapper(Shader* obj) : object(obj) {}
		template<typename T> void operator()(T)
		{
			QString type = DataTraitsList::getTraitOf<T>()->typeName();
			loadValueFuncPtr loadPtr = &Shader::loadValue<T>;
			object->m_loadValueFunctions[type] = loadPtr;

			copyValueFuncPtr copyPtr = &Shader::copyValue<T>;
			object->m_copyValueFunctions[type] = copyPtr;
		}
	};

	template<class T>
	void loadValue(QDomElement& elem)
	{
		QString name = elem.attribute("name");
		T value;
		DataTraitsList::getTraitOf<T>()->readValue(elem, &value);
		setUniform(name, value);
	}

	template<class T>
	void copyValue(QString name, const void* value)
	{
		setUniform(name, *static_cast<const T*>(value));
	}

	QVector<QPair<QString, GLuint>> m_customTextures;
};

} // namespace types

} // namespace panda

#endif // TYPES_SHADER_H
