#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

typedef boost::mpl::vector<int, PReal, QString> formatTypes;

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

		int typeOfValue = types::DataTypeId::getFullTypeOfVector(0);	// Create a copy of the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfValue,
											 true, false,
											 "input",
											 "Value to use in the format operation"));
		setupGenericObject(generic, defList);
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
		typedef Data< QVector<T> > ValueData;
		ValueData* dataInput = dynamic_cast<ValueData*>(list[0]);
		Q_ASSERT(dataInput);

		const QVector<T>& inVal = dataInput->getValue();
		int nb = inVal.size();
		int prevNb = tempList.size();

		if(!prevNb)
			tempList.fill(format.getValue(), nb);
		else if(prevNb == 1 && nb > 1)
			tempList.fill(tempList[0], nb);
		else if(nb == 1 && prevNb > 1)
			nb = 1;
		else
			tempList.resize(qMin(prevNb, nb));
		int size = tempList.size();

		for(int i=0; i<size; ++i)
			tempList[i] = tempList[i].arg(inVal[i%nb]);
	}

protected:
	Data<QString> format;
	Data< QVector<QString> > text;
	QVector<QString> tempList;
	GenericVectorData generic;
};

int GeneratorText_FormatClass = RegisterObject<GeneratorText_Format>("Generator/Text/Format").setDescription("Create a text by replacing markers by input values");

} // namespace Panda
