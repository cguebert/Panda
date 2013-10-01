#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

typedef boost::mpl::vector<int, double, QString> formatTypes;

class GeneratorText_Format : public GenericObject
{
	GENERIC_OBJECT(GeneratorText_Format, formatTypes)
public:
	PANDA_CLASS(GeneratorText_Format, GenericObject)

	GeneratorText_Format(PandaDocument *doc)
		: GenericObject(doc)
		, format(initData(&format, "format", "Format used to create the text"))
		, text(initData(&text, "text", "Text obtained by the format operation"))
		, generic(initData(&generic, "input", "Connect here the values to use in the format operation"))
	{
		addInput(&format);
		addInput(&generic);

		addOutput(&text);

		int typeOfValue = BaseData::getFullTypeOfSingleValue(0);	// Create a copy of the data connected
		GenericDataDefinitionList defList;
		defList.append(GenericDataDefinition(typeOfValue,
											 true, false,
											 "input",
											 "Value to use in the format operation"));
		setupGenericObject(&generic, defList);
	}

	void update()
	{
		tempText = format.getValue();
		GenericObject::update();
		text.setValue(tempText);

		this->cleanDirty();
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data<T> ValueData;
		ValueData* dataInput = dynamic_cast<ValueData*>(list[0]);
		Q_ASSERT(dataInput);

		const T& inVal = dataInput->getValue();

		tempText = tempText.arg(inVal);
	}

protected:
	Data<QString> format, text;
	QString tempText;
	GenericSingleValueData generic;
};

int GeneratorText_FormatClass = RegisterObject("Generator/Text/Format").setClass<GeneratorText_Format>().setDescription("Create a text by replacing markers by input values");

} // namespace Panda
