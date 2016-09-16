#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

class ListReverse : public GenericObject
{
public:
	PANDA_CLASS(ListReverse, GenericObject)

	ListReverse(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the list whose items to reverse"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "input",
							 "List to reverse");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "output",
							 "Reversed list");
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		assert(dataInput && dataOutput);

		const std::vector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();

		int size = valIn.size();
		valOut.resize(size);
		for(int i=0; i<size; ++i)
			valOut[i] = valIn[size-1-i];
	}

protected:
	GenericVectorData generic;
};

int ListReverseClass = RegisterObject<ListReverse>("List/Reverse").setDescription("Reverse the order of items in a list");

} // namespace Panda
