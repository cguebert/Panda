#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>

namespace panda {

class ExtractHead : public GenericObject
{
public:
	PANDA_CLASS(ExtractHead, GenericObject)

	ExtractHead(PandaDocument *doc)
		: GenericObject(doc)
		, value(initData(1, "head", "This number of items will be extracted from the start of the list"))
		, generic(initData("input", "Connect here the first list"))
	{
		addInput(value);
		addInput(generic);

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
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		assert(dataInput && dataOutput);

		int val = value.getValue();
		const std::vector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();

		if(val >= static_cast<int>(valIn.size()))
			valOut = valIn;
		else
			valOut.wref().assign(valIn.begin(), valIn.begin() + val);
	}

protected:
	Data< int > value;
	GenericVectorData generic;
};

int ExtractHeadClass = RegisterObject<ExtractHead>("List/Get head").setDescription("Get the first items of a list");

} // namespace Panda
