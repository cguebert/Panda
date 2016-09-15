#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>

#include <panda/types/Shader.h>

namespace panda {

using types::Color;
using types::Point;
using types::Shader;
using types::ImageWrapper;
using types::DataTypeId;

typedef helper::push_back<types::shaderValuesTypes, types::ImageWrapper> uniformTypes;

class ModifierShader_SetUniform : public GenericObject
{
public:
	PANDA_CLASS(ModifierShader_SetUniform, GenericObject)

	ModifierShader_SetUniform(PandaDocument *doc)
		: GenericObject(doc)
		, input(initData("input", "Input shader"))
		, output(initData("output", "Output shader"))
		, generic(initData("value", "Connect here the values to add to the shader"))
	{
		addInput(input);
		addInput(generic);

		addOutput(output);

		input.setWidgetData("Vertex;Fragment");
		output.setWidgetData("Vertex;Fragment");

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.emplace_back(0,	// We keep the exact type of the connected Data
							 GenericDataDefinition::Input,
							 "value",
							 "Value to set in the shader");
		// Create a single string value
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<std::string>()),
							 GenericDataDefinition::Input,
							 "name",
							 "Name of the uniform in the shader");
		setupGenericObject<uniformTypes>(this, generic, defList);
	}

	void update()
	{
		output.setValue(input.getValue());
		GenericObject::update();
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< T > ValueData;
		typedef Data< std::string > StringData;
		ValueData* dataValue = dynamic_cast<ValueData*>(list[0]);
		StringData* dataName = dynamic_cast<StringData*>(list[1]);

		assert(dataValue && dataName);

		const T& inVal = dataValue->getValue();
		const std::string name = dataName->getValue();

		output.getAccessor()->setUniform(name, inVal);
	}

protected:
	Data<Shader> input, output;
	GenericSpecificData generic;
};

int ModifierShader_SetUniformClass = RegisterObject<ModifierShader_SetUniform>("Generator/Shader/Set uniforms")
		.setDescription("Set the values of the uniforms of a shader");

} // namespace Panda
