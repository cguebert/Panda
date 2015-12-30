#include <panda/types/Shader.h>
#include <panda/helper/ShaderCache.h>
#include <panda/helper/typeList.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/system/FileRepository.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QFile>

#include <functional>
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

void Shader::setSource(QOpenGLShader::ShaderType type, const std::string& sourceCode)
{
	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = sourceCode;
	shaderSource.hash = std::hash<std::string>()(sourceCode);
	m_sourcesMap[type] = shaderSource;
}

void Shader::setSourceFromFile(QOpenGLShader::ShaderType type, const std::string& fileName)
{
	auto contents = helper::system::DataRepository.loadFile(fileName);
	if (contents.empty()) {
		std::cerr << "Shader: Unable to open file" << fileName;
		return;
	}

	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = contents;
	shaderSource.hash = std::hash<std::string>()(contents);
	m_sourcesMap[type] = shaderSource;
}

void Shader::removeSource(QOpenGLShader::ShaderType type)
{
	m_sourcesMap.erase(type);
}

bool Shader::apply(QOpenGLShaderProgram& program) const
{
	auto currentShaders = program.shaders();
	std::vector<QOpenGLShader*> newShaders;
	helper::ShaderCache* shaderCache = helper::ShaderCache::getInstance();

	// Get shader pointers from the cache
	for(const auto& source : m_sourcesMap)
		newShaders.push_back(shaderCache->getShader(source.second.type, source.second.sourceCode, source.second.hash));

	bool needLink = false;
	// Removing from the program the shader we do not want
	for(QOpenGLShader* shader : currentShaders)
	{
		if(!helper::contains(newShaders, shader))
		{
			program.removeShader(shader);
			needLink = true;
		}
	}

	// Adding the one the program does not have yet
	for(QOpenGLShader* shader : newShaders)
	{
		if (!helper::contains(currentShaders, shader))
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

			int loc = program.uniformLocation(QString::fromStdString(m_customTextures[i].first));
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

const std::vector<Shader::ShaderSource> Shader::getSources() const
{
	std::vector<Shader::ShaderSource> sources;
	for (const auto& source : m_sourcesMap)
		sources.push_back(source.second);
	return sources;
}

const Shader::ValuesVector& Shader::getValues() const
{
	return m_shaderValues;
}

void Shader::loadValue(std::string type, XmlElement& elem)
{
	if (m_loadValueFunctions.count(type))
		(this->*m_loadValueFunctions.at(type))(elem);
}

void Shader::copyValue(std::string type, std::string name, const void* value)
{
	if (m_copyValueFunctions.count(type))
		(this->*m_copyValueFunctions.at(type))(name, value);
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
{ program.setUniformValue(program.uniformLocation(QString::fromStdString(m_name)), m_value); }

template<> void ShaderValue<PReal>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValue(program.uniformLocation(QString::fromStdString(m_name)), (float)m_value); }

template<> void ShaderValue<Color>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), m_value.data(), 1, 4); }

template<> void ShaderValue<Point>::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), m_value.data(), 1, 2); }

template<> void ShaderValue< std::vector<int> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{ program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), m_value.data(), m_value.size()); }

template<> void ShaderValue< std::vector<PReal> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{
	int nb = m_value.size();
#ifdef PANDA_DOUBLE
	std::vector<float> copy(nb);
	for(int i++; i<nb; ++i)
		copy[i] = m_value[i]:
	program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), copy.data(), nb, 1);
#else
	program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), m_value.data(), nb, 1);
#endif
}

template<> void ShaderValue< std::vector<Color> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{
	if(!m_value.empty())
		program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), m_value[0].data(), m_value.size(), 4);
}

template<> void ShaderValue< std::vector<Point> >::apply(QOpenGLShaderProgram& program, const Shader&) const
{
	if(!m_value.empty())
		program.setUniformValueArray(program.uniformLocation(QString::fromStdString(m_name)), m_value[0].data(), m_value.size(), 2);
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

template<> PANDA_CORE_API std::string DataTrait<Shader>::valueTypeName() { return "shader"; }

template<>
PANDA_CORE_API void DataTrait<Shader>::writeValue(XmlElement& elem, const Shader& v)
{
	auto sources = v.getSources();

	for(const auto& source : sources)
	{
		auto sourceNode = elem.addChild("Source");
		sourceNode.setAttribute("type", (int)source.type);
		sourceNode.setText(source.sourceCode);
	}

	const auto& values = v.getValues();
	for(const auto& value : values)
	{
		auto valueNode = elem.addChild("Uniform");
		valueNode.setAttribute("name", value->getName());
		valueNode.setAttribute("type", value->dataTrait()->typeName());

		value->dataTrait()->writeValue(valueNode, value->getValue());
	}
}

template<>
PANDA_CORE_API void DataTrait<Shader>::readValue(XmlElement& elem, Shader& v)
{
	v.clear();

	auto sourceNode = elem.firstChild("Source");
	while(sourceNode)
	{
		int type = sourceNode.attribute("type").toInt();
		v.setSource(QOpenGLShader::ShaderType(type), sourceNode.text());
		sourceNode = sourceNode.nextSibling("Source");
	}

	auto valueNode = elem.firstChild("Uniform");
	while(valueNode)
	{
		std::string type = valueNode.attribute("type").toString();
		v.loadValue(type, valueNode);
		valueNode = valueNode.nextSibling("Uniform");
	}
}

//****************************************************************************//

template class Data<Shader>;
template class Data< std::vector<Shader> >;

int shaderDataClass = RegisterData< Shader >();
int shaderVectorDataClass = RegisterData< std::vector<Shader> >();

} // namespace types

} // namespace panda
