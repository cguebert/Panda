#include <panda/graphics/ShaderProgram.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

namespace panda
{

namespace graphics
{

using namespace std;

ShaderProgramId::ShaderProgramId(unsigned int id) 
	: m_id(id) 
{ }

ShaderProgramId::~ShaderProgramId() 
{ glDeleteProgram(m_id); }

unsigned int ShaderProgramId::id() const
{ return m_id; }

//****************************************************************************//

ShaderId::ShaderId(unsigned int id) 
	: m_id(id) 
{ }

ShaderId::~ShaderId() 
{ glDeleteShader(m_id); }

unsigned int ShaderId::id() const
{ return m_id; }

//****************************************************************************//

bool ShaderProgram::addShaderFromMemory(ShaderType type, const std::string& content)
{
	auto id = compileShader(type, content);
	if (id)
		addShader(type, id);

	return id != nullptr;
}

bool ShaderProgram::addShaderFromFile(ShaderType type, const std::string& path)
{
	string content;
	ifstream file;

	file.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		// Open files
		file.open(path);
		std::stringstream stream;
		stream << file.rdbuf();
		file.close();
		content = stream.str();
	}
	catch (ifstream::failure e)
	{
		cout << "Error : Cannot load shader file " << path << endl;
		return false;
	}
	
	return addShaderFromMemory(type, content);
}

void ShaderProgram::addShader(ShaderType type, ShaderId::SPtr id)
{
	auto it = std::find_if(m_shaders.begin(), m_shaders.end(), [type](const ShaderPair& s){
		return s.first == type;
	});

	if (it != m_shaders.end())
		it->second = id;
	else
		m_shaders.emplace_back(type, id);
}

ShaderId::SPtr ShaderProgram::compileShader(ShaderType type, const std::string& content)
{
	GLuint glType = 0;
	switch (type)
	{
	case ShaderType::Vertex:	glType = GL_VERTEX_SHADER;		break;
	case ShaderType::Fragment:	glType = GL_FRAGMENT_SHADER;	break;
	case ShaderType::Geometry:	glType = GL_GEOMETRY_SHADER;	break;
	case ShaderType::TessellationControl:		glType = GL_TESS_CONTROL_SHADER;		break;
	case ShaderType::TessellationEvaluation:	glType = GL_TESS_EVALUATION_SHADER;		break;
	case ShaderType::Compute:	glType = GL_COMPUTE_SHADER;	break;
	}

	GLuint shader = glCreateShader(glType);
	const GLchar* code = content.c_str();
	glShaderSource(shader, 1, &code, nullptr);
	glCompileShader(shader);

	// Print compile errors if any
	GLint success = 0;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Error : Compilation of shader failed\n" << infoLog << std::endl;
		glDeleteShader(shader);
		return nullptr;
	};

	return std::make_shared<ShaderId>(shader);
}

bool ShaderProgram::link()
{
	m_programId.reset();

	unsigned int program = glCreateProgram();
	for (const auto& shader : m_shaders)
		glAttachShader(program, shader.second->id());
	glLinkProgram(program);

	// Print linking errors if any
	GLint success = 0;
	GLchar infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "Error : Shader program link failed\n" << infoLog << std::endl;
		glDeleteProgram(program);
		return false;
	}

	m_programId = std::make_shared<ShaderProgramId>(program);
	m_shaders.clear();

	return true;
}

bool ShaderProgram::isLinked()
{ return m_programId != nullptr; }

void ShaderProgram::clear()
{
	m_shaders.clear();
	m_programId.reset();
}

unsigned int ShaderProgram::id() const
{
	if (m_programId)
		return m_programId->id();
	return 0;
}

void ShaderProgram::bind() const
{ glUseProgram(id()); }

void ShaderProgram::release() const
{ glUseProgram(0); }

int ShaderProgram::uniformLocation(const char* name) const
{ return glGetUniformLocation(id(), name); }

void ShaderProgram::setUniformValue(int location, int value) const
{ glUniform1i(location, value); }

void ShaderProgram::setUniformValue(int location, float value) const
{ glUniform1fv(location, 1, &value); }

void ShaderProgram::setUniformValue(int location, const std::vector<int>& value) const
{ glUniform1iv(location, value.size(), value.data()); }

void ShaderProgram::setUniformValue(int location, const std::vector<float>& value) const
{ glUniform1fv(location, value.size(), value.data()); }

void ShaderProgram::setUniformValueArray(int location, const float* values, int count, int tupleSize) const
{
	switch (tupleSize) {
		case 1: glUniform1fv(location, count, values); break;
		case 2: glUniform2fv(location, count, values); break;
		case 3: glUniform3fv(location, count, values); break;
		case 4: glUniform4fv(location, count, values); break;
		default: std::cerr << "ShaderProgram::setUniformValueArray: size " << tupleSize << "not supported" << std::endl;
	}
}

void ShaderProgram::setUniformValueMat4(int location, const float* value) const
{ glUniformMatrix4fv(location, 1, GL_FALSE, value); }

void ShaderProgram::setUniformValue(const char* name, int value) const
{ setUniformValue(uniformLocation(name), value); }

void ShaderProgram::setUniformValue(const char* name, float value) const
{ setUniformValue(uniformLocation(name), value); }

void ShaderProgram::setUniformValue(const char* name, const std::vector<int>& value) const
{ setUniformValue(uniformLocation(name), value); }

void ShaderProgram::setUniformValue(const char* name, const std::vector<float>& value) const
{ setUniformValue(uniformLocation(name), value); }

void ShaderProgram::setUniformValueArray(const char* name, const float* values, int count, int tupleSize) const
{ setUniformValueArray(uniformLocation(name), values, count, tupleSize); }

void ShaderProgram::setUniformValueMat4(const char* name, const float* value) const
{ setUniformValueMat4(uniformLocation(name), value); }

int ShaderProgram::attributeLocation(const char* name) const
{ return glGetAttribLocation(id(), name); }

void ShaderProgram::enableAttributeArray(int location) const
{ glEnableVertexAttribArray(location); }

void ShaderProgram::disableAttributeArray(int location) const
{ glDisableVertexAttribArray(location); }

void ShaderProgram::enableAttributeArray(const char* name) const
{ enableAttributeArray(attributeLocation(name)); }

void ShaderProgram::disableAttributeArray(const char* name) const
{ disableAttributeArray(attributeLocation(name)); }

void ShaderProgram::setAttributeArray(int location, const float* values, int tupleSize, int stride)
{ glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE, stride, values); }

void ShaderProgram::setAttributeArray(int location, GLenum type, const void* values, int tupleSize, int stride)
{ glVertexAttribPointer(location, tupleSize, type, GL_FALSE, stride, values); }

void ShaderProgram::setAttributeArray(const char* name, const float* values, int tupleSize, int stride)
{ setAttributeArray(attributeLocation(name), values, tupleSize, stride); }

void ShaderProgram::setAttributeArray(const char* name, GLenum type, const void* values, int tupleSize, int stride)
{ setAttributeArray(attributeLocation(name), type, values, tupleSize, stride); }

} // namespace graphics

} // namespace panda
