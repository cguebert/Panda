#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

class FilterItems : public GenericObject
{
public:
	PANDA_CLASS(FilterItems, GenericObject)

	FilterItems(PandaDocument *doc)
		: GenericObject(doc)
		, condition(initData("condition", "Boolean values specifying which items are kept"))
		, generic(initData("input", "Connect here the list"))
	{
		addInput(condition);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "input",
							 "The initial list of items");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "output",
							 "Result of the deletion");
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
		const std::vector<int> &valCond = condition.getValue();

		int nbIn = valIn.size(), nbCond = valCond.size();
		int size = std::min(nbIn, nbCond);
		valOut.clear();
		for(int i=0; i<size; ++i)
		{
			if(valCond[i])
				valOut.push_back(valIn[i]);
		}
	}

protected:
	Data< std::vector<int> > condition;
	GenericVectorData generic;
};

int FilterItemsClass = RegisterObject<FilterItems>("List/Filter items").setDescription("Remove items for which the condition is 0");

} // namespace Panda
