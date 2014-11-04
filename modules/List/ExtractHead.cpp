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
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List of items from which to extract the head"));
		defList.push_back(GenericDataDefinition(listType,
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
		auto valOut = dataOutput->getAccessor();

		if(val >= valIn.size())
			valOut = valIn;
		else
			valOut = valIn.mid(0, val);
	}

protected:
	Data< int > value;
	GenericVectorData generic;
};

int ExtractHeadClass = RegisterObject<ExtractHead>("List/Get head").setDescription("Get the first items of a list");

} // namespace Panda