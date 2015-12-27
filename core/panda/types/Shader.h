#ifndef TYPES_SHADER_H
#define TYPES_SHADER_H

#include <panda/core.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/DataTraits.h>

#include <QString>
#include <QOpenGLShader>
#include <QDomElement>

#include <map>
#include <memory>
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
	void apply(QOpenGLShaderProgram& program, const Shader& shader) const override;
	void cleanup() const override {}
	QString getName() const override 
	{ return m_name; }
	AbstractDataTrait* dataTrait() const override
	{ return DataTraitsList::getTraitOf<T>(); }
	const void* getValue() const override
	{ return &m_value; }
	void* getValue() override
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
extern template class PANDA_CORE_API ShaderValue<std::vector<int>>;
extern template class PANDA_CORE_API ShaderValue<std::vector<PReal>>;
extern template class PANDA_CORE_API ShaderValue<std::vector<Color>>;
extern template class PANDA_CORE_API ShaderValue<std::vector<Point>>;
#endif

//****************************************************************************//

typedef std::tuple<int, PReal, Color, Point,
	std::vector<int>, std::vector<PReal>, std::vector<Color>, std::vector<Point> > shaderValuesTypes;

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

	const std::vector<ShaderSource> getSources() const;

	template<class T>
	void setUniform(QString name, const T& value)
	{
		m_shaderValues.push_back(std::make_shared<ShaderValue<T>>(name, value));
	}

	template<>
	void setUniform(QString name, const ImageWrapper& img)
	{
		m_customTextures.push_back(std::make_pair(name, img.getTextureId()));
	}

	typedef std::vector<std::shared_ptr<BaseShaderValue>> ValuesVector;
	const ValuesVector& getValues() const;

	void loadValue(QString type, QDomElement &elem);
	void copyValue(QString type, QString name, const void* value);

	bool operator==(const Shader& s) const;
	bool operator!=(const Shader& s) const;

protected:
	typedef std::map<QOpenGLShader::ShaderType, ShaderSource> SourcesMap;
	SourcesMap m_sourcesMap;

	ValuesVector m_shaderValues;

	typedef void(Shader::*loadValueFuncPtr)(QDomElement&);
	std::map<QString, loadValueFuncPtr> m_loadValueFunctions;

	typedef void(Shader::*copyValueFuncPtr)(QString, const void*);
	std::map<QString, copyValueFuncPtr> m_copyValueFunctions;

	struct functionCreatorWrapper
	{
		Shader* object;
		functionCreatorWrapper(Shader* obj) : object(obj) {}
		template<typename T> void operator()(T)
		{
			QString type = DataTraitsList::getTraitOf<T>()->typeName();
			loadValueFuncPtr loadPtr = &Shader::loadValue<T>;
			object->m_loadValueFunctions.emplace(type, loadPtr);

			copyValueFuncPtr copyPtr = &Shader::copyValue<T>;
			object->m_copyValueFunctions.emplace(type, copyPtr);
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

	std::vector<std::pair<QString, GLuint>> m_customTextures;
};

} // namespace types

} // namespace panda

#endif // TYPES_SHADER_H
