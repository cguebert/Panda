#ifndef GRAPHICS_SHADERPROGRAM_H
#define GRAPHICS_SHADERPROGRAM_H

#include <panda/core.h>

#include <memory>
#include <string>
#include <vector>

namespace panda
{

namespace graphics
{

enum class ShaderType : char
{ Vertex = 1, Fragment, Geometry, TessellationControl, TessellationEvaluation, Compute };

// To free the shader only when every copy is destroyed
class PANDA_CORE_API ShaderId
{
public:
	using SPtr = std::shared_ptr<ShaderId>;
	ShaderId(ShaderType type, unsigned int id);
	~ShaderId();

	unsigned int id() const;
	ShaderType type() const;

private:
	unsigned int m_id;
	ShaderType m_type;
};

// To free the shader program only when every copy is destroyed
class PANDA_CORE_API ShaderProgramId
{
public:
	using SPtr = std::shared_ptr<ShaderProgramId>;
	ShaderProgramId(unsigned int id = 0);
	~ShaderProgramId();

	unsigned int id() const;

private:
	unsigned int m_id;
};

//****************************************************************************//

class PANDA_CORE_API ShaderProgram
{
public:
	ShaderProgram(ShaderProgramId::SPtr id = nullptr);

	bool addShaderFromMemory(ShaderType type, const std::string& content);
	bool addShaderFromFile(ShaderType type, const std::string& path);
	void addShader(ShaderId::SPtr id);

	static ShaderId::SPtr compileShader(ShaderType type, const std::string& content, std::string* errorString = nullptr);

	bool link(std::string* errorString = nullptr);
	bool isLinked();

	void clear(); // Remove shaders

	unsigned int id() const;
	ShaderProgramId::SPtr getProgramId() const;

	void bind() const;
	void release() const;

	int uniformLocation(const char* name) const;

	void setUniformValue(int location, int value) const;
	void setUniformValue(int location, float value) const;
	void setUniformValue(int location, const std::vector<int>& value) const;
	void setUniformValue(int location, const std::vector<float>& value) const;
	void setUniformValueArray(int location, const float* values, int count, int tupleSize) const;
	void setUniformValueMat4(int location, const float* value) const;

	void setUniformValue(const char* name, int value) const;
	void setUniformValue(const char* name, float value) const;
	void setUniformValue(const char* name, const std::vector<int>& value) const;
	void setUniformValue(const char* name, const std::vector<float>& value) const;
	void setUniformValueArray(const char* name, const float* values, int count, int tupleSize) const;
	void setUniformValueMat4(const char* name, const float* value) const;

	int attributeLocation(const char* name) const;

protected:
	std::vector<ShaderId::SPtr> m_shaders;
	ShaderProgramId::SPtr m_programId;
};

//****************************************************************************//

inline ShaderId::ShaderId(ShaderType type, unsigned int id) 
	: m_type(type), m_id(id) {}

inline unsigned int ShaderId::id() const
{ return m_id; }

inline ShaderType ShaderId::type() const
{ return m_type; }

//****************************************************************************//

inline ShaderProgramId::ShaderProgramId(unsigned int id) 
	: m_id(id) {}

inline unsigned int ShaderProgramId::id() const
{ return m_id; }

//****************************************************************************//

inline ShaderProgram::ShaderProgram(ShaderProgramId::SPtr id)
	: m_programId(id) {}

inline ShaderProgramId::SPtr ShaderProgram::getProgramId() const
{ return m_programId; }

inline bool ShaderProgram::isLinked()
{ return m_programId != nullptr; }

inline unsigned int ShaderProgram::id() const
{ return m_programId ? m_programId->id() : 0; }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_SHADERPROGRAM_H
