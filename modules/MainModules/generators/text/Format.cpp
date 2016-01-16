#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/algorithm.h>

namespace
{

template <class T> std::string convert(const T& value)
{ return std::to_string(value); }

std::string convert(const std::string& value)
{ return value; }

}

namespace panda {

typedef std::tuple<int, PReal, std::string> formatTypes;

class GeneratorText_Format : public GenericObject
{
public:
	PANDA_CLASS(GeneratorText_Format, GenericObject)

	GeneratorText_Format(PandaDocument *doc)
		: GenericObject(doc)
		, format(initData("format", "Format used to create the text"))
		, text(initData("text", "Text obtained by the format operation"))
		, generic(initData("input", "Connect here the values to use in the format operation"))
	{
		addInput(format);
		addInput(generic);

		addOutput(text);

		int typeOfValue = types::DataTypeId::getFullTypeOfVector(0);	// Create a copy of the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfValue,
											 true, false,
											 "input",
											 "Value to use in the format operation"));
		setupGenericObject<formatTypes>(this, generic, defList);
	}

	void update()
	{
		tempList.clear();
		GenericObject::update();
		text.setValue(tempList);

		cleanDirty();
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ValueData;
		ValueData* dataInput = dynamic_cast<ValueData*>(list[0]);
		assert(dataInput);

		const std::vector<T>& inVal = dataInput->getValue();
		int nb = inVal.size();
		int prevNb = tempList.size();

		if(!prevNb)
			tempList.resize(nb, format.getValue());
		else if(prevNb == 1 && nb > 1)
			tempList.resize(nb, tempList[0]);
		else if(nb == 1 && prevNb > 1)
			nb = 1;
		else
			tempList.resize(std::min(prevNb, nb));
		int size = tempList.size();

		for (int i = 0; i < size; ++i)
		{
			std::string key = "%" + std::to_string(i);
			helper::replaceAll(tempList[i], key, convert(inVal[i%nb]));
		}
	}

protected:
	Data<std::string> format;
	Data< std::vector<std::string> > text;
	std::vector<std::string> tempList;
	GenericVectorData generic;
};

int GeneratorText_FormatClass = RegisterObject<GeneratorText_Format>("Generator/Text/Format").setDescription("Create a text by replacing markers by input values");

} // namespace Panda
