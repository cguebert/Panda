#include <panda/types/Shader.h>
#include <panda/helper/ShaderCache.h>
#include <panda/helper/typeList.h>
#include <panda/helper/system/FileRepository.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QFile>

#include <iostream>

namespace panda
{

namespace types
{

Shader::Shader()
{
	helper::for_each_type<shaderValuesTypes>(functionCreatorWrapper(this));
}

Shader& Shader::operator=(const Shader& shader)
{
	clear();
	m_sourcesMap = shader.m_sourcesMap;
	auto values = shader.getValues();
	for(auto value : values)
		copyValue(value->dataTrait()->typeName(), value->getName(), value->getValue());

	// We don't copy the textures, it is by design (only SetShaderValues can set textures)

	return *this;
}

void Shader::clear()
{
	m_sourcesMap.clear();
	m_shaderValues.clear();
	m_customTextures.clear();
}

void Shader::setSource(QOpenGLShader::ShaderType type, const QString& sourceCode)
{
	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = sourceCode.toLatin1();
	shaderSource.hash = qHash(shaderSource.sourceCode);
	m_sourcesMap[type] = shaderSource;
}

void Shader::setSourceFromFile(QOpenGLShader::ShaderType type, const QString& fileName)
{
	QByteArray contents = helper::system::DataRepository.loadFile(fileName);
	if (contents.isEmpty()) {
		qWarning() << "Shader: Unable to open file" << fileName;
		return;
	}

	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = contents;
	shaderSource.hash = qHash(shaderSource.sourceCode);
	m_sourcesMap[type] = shaderSource;
}

void Shader::removeSource(QOpenGLShader::ShaderType type)
{
	m_sourcesMap.remove(type);
}

bool Shader::apply(QOpenGLShaderProgram& program) const
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
			value->apply(program, *this);

		// Register custom textures
		QOpenGLFunctions glFuncs(QOpenGLContext::currentContext());
		for(unsigned int i=0, nb=m_customTextures.size(); i<nb; ++i)
		{
			GLuint id = m_customTextures[i].second;
			if(!id)
				continue;

			int loc = program.uniformLocation(m_customTextures[i].first);
			if(loc == -1)
				continue;

			glFuncs.glActiveTexture(GL_TEXTURE8 + i);
			glBindTexture(GL_TEXTURE_2D, m_customTextures[i].second);
			program.setUniformValue(loc, 8 + i);
			glFuncs.glActiveTexture(0);
		}
	}
	else
		return false;

	return true;
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

bool Shader::operator==(const Shader& shader) const
{
	return m_sourcesMap == shader.m_sourcesMap
		&& m_shaderValues == shader.m_shaderValues
		&& m_customTextures == shader.m_customTextures;
}

bool Shader::operator!=(const Shader& shader) const
{
	return !(*this == shader);
}

//****************************************************************************//

template<> void ShaderValue<int>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValue(program.uniformLocation(m_name), m_value); }

template<> void ShaderValue<PReal>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValue(program.uniformLocation(m_name), (float)m_value); }

template<> void ShaderValue<Color>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValueArray(program.uniformLocation(m_name), m_value.data(), 1, 4); }

template<> void ShaderValue<Point>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValueArray(program.uniformLocation(m_name), m_value.data(), 1, 2); }

template<> void ShaderValue< std::vector<int> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValueArray(program.uniformLocation(m_name), m_value.data(), m_value.size()); }

template<> void ShaderValue< std::vector<PReal> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{
	int nb = m_value.size();
#ifdef PANDA_DOUBLE
	std::vector<float> copy(nb);
	for(int i++; i<nb; ++i)
		copy[i] = m_value[i]:
	program.setUniformValueArray(program.uniformLocation(m_name), copy.data(), nb, 1);
#else
	program.setUniformValueArray(program.uniformLocation(m_name), m_value.data(), nb, 1);
#endif
}

template<> void ShaderValue< std::vector<Color> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{
	if(!m_value.empty())
		program.setUniformValueArray(program.uniformLocation(m_name), m_value[0].data(), m_value.size(), 4);
}

template<> void ShaderValue< std::vector<Point> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{
	if(!m_value.empty())
		program.setUniformValueArray(program.uniformLocation(m_name), m_value[0].data(), m_value.size(), 2);
}

template class PANDA_CORE_API ShaderValue<int>;
template class PANDA_CORE_API ShaderValue<PReal>;
template class PANDA_CORE_API ShaderValue<Color>;
template class PANDA_CORE_API ShaderValue<Point>;
template class PANDA_CORE_API ShaderValue<std::vector<int>>;
template class PANDA_CORE_API ShaderValue<std::vector<PReal>>;
template class PANDA_CORE_API ShaderValue<std::vector<Color>>;
template class PANDA_CORE_API ShaderValue<std::vector<Point>>;

//****************************************************************************//

template<> PANDA_CORE_API QString DataTrait<Shader>::valueTypeName() { return "shader"; }

template<>
PANDA_CORE_API void DataTrait<Shader>::writeValue(QDomDocument& doc, QDomElement& elem, const Shader& v)
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
		valueNode.setAttribute("type", value->dataTrait()->typeName());
		elem.appendChild(valueNode);

		value->dataTrait()->writeValue(doc, valueNode, value->getValue());
	}
}

template<>
PANDA_CORE_API void DataTrait<Shader>::readValue(QDomElement& elem, Shader& v)
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

//****************************************************************************//

template class Data<Shader>;
template class Data< std::vector<Shader> >;

int shaderDataClass = RegisterData< Shader >();
int shaderVectorDataClass = RegisterData< std::vector<Shader> >();

} // namespace types

} // namespace panda
