#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

class ExtractTail : public GenericObject
{
public:
	PANDA_CLASS(ExtractTail, GenericObject)

	ExtractTail(PandaDocument *doc)
		: GenericObject(doc)
		, value(initData(1, "tail", "All items starting at this index will be extracted. If negative, extract this number of items instead."))
		, generic(initData("input", "Connect here the first list"))
	{
		addInput(value);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "input",
							 "List of items from which to extract the tail");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "output",
							 "Result of the extraction");
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
		int nb = valIn.size();

		if (val >= nb)
			valOut.clear();
		else if (val > 0)
			valOut.wref().assign(valIn.begin() + val, valIn.end());
		else
			valOut.wref().assign(valIn.begin() + std::max<int>(0, valIn.size() + val), valIn.end());
	}

protected:
	Data< int > value;
	GenericVectorData generic;
};

int ExtractTailClass = RegisterObject<ExtractTail>("List/Get tail").setDescription("Get the last items of a list");

} // namespace Panda
