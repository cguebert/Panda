#ifndef TYPES_SHADER_H
#define TYPES_SHADER_H

#include <panda/core.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/DataTraits.h>

#include <map>
#include <memory>
#include <string>
#include <tuple>

namespace panda
{

namespace graphics
{ 
	class ShaderProgram; 
}

namespace types
{

class Shader;

class BaseShaderValue
{
public:
	virtual void apply(graphics::ShaderProgram& program) const = 0; // Add the value to the shader program
	virtual void cleanup() const = 0;	// Disable the array from the shader after rendering
	virtual const std::string& getName() const = 0;
	virtual AbstractDataTrait* dataTrait() const = 0;
	virtual const void* getValue() const = 0;
	virtual void* getValue() = 0;
};

template<class T>
class ShaderValue : public BaseShaderValue
{
public:
	ShaderValue(std::string name, const T& val) : m_name(name), m_value(val) { }
	void apply(graphics::ShaderProgram& program) const override;
	void cleanup() const override {}
	const std::string& getName() const override 
	{ return m_name; }
	AbstractDataTrait* dataTrait() const override
	{ return DataTraitsList::getTraitOf<T>(); }
	const void* getValue() const override
	{ return &m_value; }
	void* getValue() override
	{ return &m_value; }

protected:
	std::string m_name;
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
	enum class ShaderType : char { Vertex = 1, Fragment, Geometry, 
		TessellationControl, TessellationEvaluation, Compute };

	Shader();

	Shader& operator=(const Shader& shader);

	void clear(); /// Remove all sources & all data
	void setSource(ShaderType type, const std::string& sourceCode);
	void setSourceFromFile(ShaderType type, const std::string& fileName);
	void removeSource(ShaderType type);

	bool apply(graphics::ShaderProgram& program) const;

	struct ShaderSource
	{
		ShaderType type;
		std::string sourceCode;
		unsigned int hash;

		bool operator==(const ShaderSource& s) const
		{ return hash == s.hash; }
	};

	const std::vector<ShaderSource> getSources() const;

	template<class T>
	void setUniform(std::string name, const T& value)
	{ m_shaderValues.push_back(std::make_shared<ShaderValue<T>>(name, value)); }

	template<>
	void setUniform(std::string name, const ImageWrapper& img)
	{ m_customTextures.push_back(std::make_pair(name, img.getTextureId())); }

	typedef std::vector<std::shared_ptr<BaseShaderValue>> ValuesVector;
	const ValuesVector& getValues() const;

	void loadValue(std::string type, XmlElement& elem);
	void copyValue(std::string type, std::string name, const void* value);

	bool operator==(const Shader& s) const;
	bool operator!=(const Shader& s) const;

protected:
	typedef std::map<ShaderType, ShaderSource> SourcesMap;
	SourcesMap m_sourcesMap;

	ValuesVector m_shaderValues;

	typedef void(Shader::*loadValueFuncPtr)(XmlElement&);
	std::map<std::string, loadValueFuncPtr> m_loadValueFunctions;

	typedef void(Shader::*copyValueFuncPtr)(std::string, const void*);
	std::map<std::string, copyValueFuncPtr> m_copyValueFunctions;

	struct functionCreatorWrapper
	{
		Shader* object;
		functionCreatorWrapper(Shader* obj) : object(obj) {}
		template<typename T> void operator()(T)
		{
			std::string type = DataTraitsList::getTraitOf<T>()->typeName();
			loadValueFuncPtr loadPtr = &Shader::loadValue<T>;
			object->m_loadValueFunctions.emplace(type, loadPtr);

			copyValueFuncPtr copyPtr = &Shader::copyValue<T>;
			object->m_copyValueFunctions.emplace(type, copyPtr);
		}
	};

	template<class T>
	void loadValue(XmlElement& elem)
	{
		std::string name = elem.attribute("name").toString();
		T value;
		DataTraitsList::getTraitOf<T>()->readValue(elem, &value);
		setUniform(name, value);
	}

	template<class T>
	void copyValue(std::string name, const void* value)
	{ setUniform(name, *static_cast<const T*>(value)); }

	std::vector<std::pair<std::string, unsigned int>> m_customTextures;
};

} // namespace types

} // namespace panda

#endif // TYPES_SHADER_H
