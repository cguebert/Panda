#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Shader.h>

#include <boost/mpl/push_back.hpp>

namespace panda {

using types::Color;
using types::Point;
using types::Shader;
using types::ImageWrapper;
using types::DataTypeId;

typedef boost::mpl::push_back<types::shaderValuesTypes, types::ImageWrapper>::type uniformTypes;

class ModifierShader_SetUniform : public GenericObject
{
	GENERIC_OBJECT(ModifierShader_SetUniform, uniformTypes)
public:
	PANDA_CLASS(ModifierShader_SetUniform, GenericObject)

	ModifierShader_SetUniform(PandaDocument *doc)
		: GenericObject(doc)
		, input(initData(&input, "input", "Input shader"))
		, output(initData(&output, "output", "Output shader"))
		, generic(initData(&generic, "value", "Connect here the values to add to the shader"))
	{
		addInput(&input);
		addInput(&generic);

		addOutput(&output);

		input.setWidgetData("Vertex;Fragment");
		output.setWidgetData("Vertex;Fragment");

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.push_back(GenericDataDefinition(0,	// We keep the exact type of the connected Data
											 true, false,
											 "value",
											 "Value to set in the shader"));
		// Create a single int value
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<QString>()),
											 true, false,
											 "name",
											 "Name of the uniform in the shader"));
		setupGenericObject(generic, defList);
	}

	void update()
	{
		output.setValue(input.getValue());
		GenericObject::update();

		cleanDirty();
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< T > ValueData;
		typedef Data< QString > StringData;
		ValueData* dataValue = dynamic_cast<ValueData*>(list[0]);
		StringData* dataName = dynamic_cast<StringData*>(list[1]);

		Q_ASSERT(dataValue && dataName);

		const T& inVal = dataValue->getValue();
		const QString name = dataName->getValue();

		output.getAccessor()->setUniform(name, inVal);
	}

protected:
	Data<Shader> input, output;
	GenericSpecificData generic;
};

int ModifierShader_SetUniformClass = RegisterObject<ModifierShader_SetUniform>("Generator/Shader/Set uniforms")
		.setDescription("Set the values of the uniforms of a shader");

} // namespace Panda
