#include <GL/glew.h>

#include <panda/types/Shader.h>
#include <panda/helper/ShaderCache.h>
#include <panda/helper/typeList.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/system/FileRepository.h>

#include <panda/data/DataFactory.h>

#include <iostream>

namespace
{

panda::graphics::ShaderType convert(panda::types::Shader::ShaderType type)
{
	return static_cast<panda::graphics::ShaderType>(static_cast<char>(type));
}

}

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

void Shader::setSource(ShaderType type, const std::string& sourceCode)
{
	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = sourceCode;
	shaderSource.hash = std::hash<std::string>()(sourceCode);
	m_sourcesMap[type] = shaderSource;
}

void Shader::setSourceFromFile(ShaderType type, const std::string& fileName)
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

void Shader::removeSource(ShaderType type)
{
	m_sourcesMap.erase(type);
}

bool Shader::apply(graphics::ShaderProgram& program) const
{
	// Get shader from the cache (compile them if necessary)
	helper::ShaderCache* shaderCache = helper::ShaderCache::getInstance();
	helper::ShaderCache::ShadersList shadersList;
	for (const auto& source : m_sourcesMap)
	{
		auto type = convert(source.second.type);
		auto id = shaderCache->getShader(type, source.second.sourceCode, source.second.hash);
		if (!id)
			return false;
		shadersList.emplace_back(type, source.second.hash);
	}

	program = shaderCache->getShaderProgram(shadersList);
		
	if(!program.isLinked())
		return false;

	program.bind();
	for(const auto& value : m_shaderValues)
		value->apply(program);

	// Register custom textures
	for(unsigned int i=0, nb=m_customTextures.size(); i<nb; ++i)
	{
		GLuint id = m_customTextures[i].second;
		if(!id)
			continue;

		int loc = program.uniformLocation(m_customTextures[i].first.c_str());
		if(loc == -1)
			continue;

		glActiveTexture(GL_TEXTURE8 + i);
		glBindTexture(GL_TEXTURE_2D, m_customTextures[i].second);
		glUniform1i(loc, 8 + i);
		glActiveTexture(GL_TEXTURE0);
	}

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

template<> void ShaderValue<int>::apply(graphics::ShaderProgram& program) const
{ program.setUniformValue(m_name.c_str(), m_value); }

template<> void ShaderValue<float>::apply(graphics::ShaderProgram& program) const
{ program.setUniformValue(m_name.c_str(), m_value); }

template<> void ShaderValue<Color>::apply(graphics::ShaderProgram& program) const
{ program.setUniformValueArray(m_name.c_str(), m_value.data(), 1, 4); }

template<> void ShaderValue<Point>::apply(graphics::ShaderProgram& program) const
{ program.setUniformValueArray(m_name.c_str(), m_value.data(), 1, 2); }

template<> void ShaderValue< std::vector<int> >::apply(graphics::ShaderProgram& program) const
{ program.setUniformValue(m_name.c_str(), m_value); }

template<> void ShaderValue< std::vector<float> >::apply(graphics::ShaderProgram& program) const
{ program.setUniformValue(m_name.c_str(), m_value); }

template<> void ShaderValue< std::vector<Color> >::apply(graphics::ShaderProgram& program) const
{
	if(!m_value.empty())
		program.setUniformValueArray(m_name.c_str(), m_value[0].data(), m_value.size(), 4);
}

template<> void ShaderValue< std::vector<Point> >::apply(graphics::ShaderProgram& program) const
{
	if(!m_value.empty())
		program.setUniformValueArray(m_name.c_str(), m_value[0].data(), m_value.size(), 2);
}

template class PANDA_CORE_API ShaderValue<int>;
template class PANDA_CORE_API ShaderValue<float>;
template class PANDA_CORE_API ShaderValue<Color>;
template class PANDA_CORE_API ShaderValue<Point>;
template class PANDA_CORE_API ShaderValue<std::vector<int>>;
template class PANDA_CORE_API ShaderValue<std::vector<float>>;
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
		v.setSource(static_cast<Shader::ShaderType>(type), sourceNode.text());
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
