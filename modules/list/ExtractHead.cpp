#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ExtractHead : public GenericObject
{
	GENERIC_OBJECT(ExtractHead, allDataTypes)
public:
	PANDA_CLASS(ExtractHead, GenericObject)

	ExtractHead(PandaDocument *doc)
		: GenericObject(doc)
		, value(initData(&value, 1, "head", "This number of items will be extracted from the start of the list"))
		, generic(initData(&generic, "input", "Connect here the first list"))
    {
		addInput(&value);
        addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = BaseData::getFullTypeOfVector(0);
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List of items from which to extract the head"));
		defList.append(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Result of the extraction"));
		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		int val = value.getValue();
		const QVector<T> &valIn = dataInput->getValue();
		QVector<T> &valOut = *dataOutput->beginEdit();

		if(val >= valIn.size())
			valOut = valIn;
		else
			valOut.swap(valIn.mid(0, val));

		dataOutput->endEdit();
	}

protected:
	Data< int > value;
	GenericVectorData generic;
};

int ExtractHeadClass = RegisterObject("List/Get head").setClass<ExtractHead>().setDescription("Get the first items of a list");

} // namespace Panda
