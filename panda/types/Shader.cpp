#include <panda/types/Shader.h>
#include <panda/helper/ShaderCache.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#include <boost/mpl/for_each.hpp>

#include <QOpenGLShaderProgram>

#include <iostream>

namespace panda
{

namespace types
{

Shader::Shader()
{
	boost::mpl::for_each<shaderValuesTypes>(functionCreatorWrapper(this));
}

Shader& Shader::operator=(const Shader& shader)
{
	clear();
	m_sourcesMap = shader.m_sourcesMap;
	auto values = shader.getValues();
	for(auto value : values)
		copyValue(value->dataTrait()->description(), value->getName(), value->getValue());

	return *this;
}

void Shader::clear()
{
	m_sourcesMap.clear();
	m_shaderValues.clear();
}

void Shader::setSource(QOpenGLShader::ShaderType type, QString sourceCode)
{
	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = sourceCode.toLatin1();
	shaderSource.hash = qHash(shaderSource.sourceCode);
	m_sourcesMap[type] = shaderSource;
}

void Shader::removeSource(QOpenGLShader::ShaderType type)
{
	m_sourcesMap.remove(type);
}

void Shader::apply(QOpenGLShaderProgram& program) const
{
	QList<QOpenGLShader*> currentShaders = program.shaders();
	QList<QOpenGLShader*> newShaders;
	helper::ShaderCache* shaderCache = helper::ShaderCache::getInstance();

	// Get shader pointers from the cache
	for(const ShaderSource& source : m_sourcesMap.values())
		newShaders.push_back(shaderCache->getShader(source.type, source.sourceCode, source.hash));

	bool needLink = false;
	// Removing from the program the shader we do not want
	for(QOpenGLShader* shader : currentShaders)
	{
		if(!newShaders.contains(shader))
		{
			program.removeShader(shader);
			needLink = true;
		}
	}

	// Adding the one the program does not have yet
	for(QOpenGLShader* shader : newShaders)
	{
		if(!currentShaders.contains(shader))
		{
			program.addShader(shader);
			needLink = true;
		}
	}

	if(needLink)
		program.link();

	if(program.isLinked())
	{
		program.bind();
		for(auto value : m_shaderValues)
			value->apply(program);
	}
}

const QList<Shader::ShaderSource> Shader::getSources() const
{
	return m_sourcesMap.values();
}

const Shader::ValuesVector& Shader::getValues() const
{
	return m_shaderValues;
}

void Shader::loadValue(QString type, QDomElement& elem)
{
	if(m_loadValueFunctions.contains(type))
		(this->*m_loadValueFunctions[type])(elem);
}

void Shader::copyValue(QString type, QString name, const void* value)
{
	if(m_copyValueFunctions.contains(type))
		(this->*m_copyValueFunctions[type])(name, value);
}

//***************************************************************//

template<> void ShaderValue<int>::apply(QOpenGLShaderProgram& program) const
{ program.setUniformValue(program.uniformLocation(m_name), m_value); }

template<> void ShaderValue<PReal>::apply(QOpenGLShaderProgram& program) const
{ program.setUniformValue(program.uniformLocation(m_name), (float)m_value); }

template<> void ShaderValue<Color>::apply(QOpenGLShaderProgram& program) const
{ program.setUniformValueArray(program.uniformLocation(m_name), m_value.ptr(), 1, 4); }

template<> void ShaderValue<Point>::apply(QOpenGLShaderProgram& program) const
{ program.setUniformValueArray(program.uniformLocation(m_name), m_value.ptr(), 1, 2); }

template<> void ShaderValue< QVector<int> >::apply(QOpenGLShaderProgram& program) const
{ program.setUniformValueArray(program.uniformLocation(m_name), m_value.data(), m_value.size()); }

template<> void ShaderValue< QVector<PReal> >::apply(QOpenGLShaderProgram& program) const
{
	int nb = m_value.size();
#ifdef PANDA_DOUBLE
	QVector<float> copy(nb);
	for(int i++; i<nb; ++i)
		copy[i] = m_value[i]:
	program.setUniformValueArray(program.uniformLocation(m_name), copy.data(), nb, 1);
#else
	program.setUniformValueArray(program.uniformLocation(m_name), m_value.data(), nb, 1);
#endif
}

template<> void ShaderValue< QVector<Color> >::apply(QOpenGLShaderProgram& program) const
{
	if(!m_value.empty())
		program.setUniformValueArray(program.uniformLocation(m_name), m_value[0].ptr(), m_value.size(), 4);
}

template<> void ShaderValue< QVector<Point> >::apply(QOpenGLShaderProgram& program) const
{
	if(!m_value.empty())
		program.setUniformValueArray(program.uniformLocation(m_name), m_value[0].ptr(), m_value.size(), 2);
}

//***************************************************************//

template<> QString DataTrait<Shader>::valueTypeName() { return "shader"; }

template<>
void DataTrait<Shader>::writeValue(QDomDocument& doc, QDomElement& elem, const Shader& v)
{
	auto sources = v.getSources();

	for(const auto& source : sources)
	{
		QDomElement sourceNode = doc.createElement("Source");
		elem.appendChild(sourceNode);
		sourceNode.setAttribute("type", (int)source.type);

		QDomText node = doc.createTextNode(source.sourceCode);
		sourceNode.appendChild(node);
	}

	const auto& values = v.getValues();
	for(const auto& value : values)
	{
		QDomElement valueNode = doc.createElement("Uniform");
		valueNode.setAttribute("name", value->getName());
		valueNode.setAttribute("type", value->dataTrait()->description());
		elem.appendChild(valueNode);

		value->dataTrait()->writeValue(doc, valueNode, value->getValue());
	}
}

template<>
void DataTrait<Shader>::readValue(QDomElement& elem, Shader& v)
{
	v.clear();

	QDomElement sourceNode = elem.firstChildElement("Source");
	while(!sourceNode.isNull())
	{
		int type = sourceNode.attribute("type").toInt();
		v.setSource(QOpenGLShader::ShaderType(type), sourceNode.text());
		sourceNode = sourceNode.nextSiblingElement("Source");
	}

	QDomElement valueNode = elem.firstChildElement("Uniform");
	while(!valueNode.isNull())
	{
		QString type = valueNode.attribute("type");
		v.loadValue(type, valueNode);
		valueNode = valueNode.nextSiblingElement("Uniform");
	}
}

//***************************************************************//

template class Data<Shader>;
template class Data< QVector<Shader> >;

int shaderDataClass = RegisterData< Shader >();
int shaderVectorDataClass = RegisterData< QVector<Shader> >();

} // namespace types

} // namespace panda
