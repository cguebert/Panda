#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Animation.h>

namespace panda {

class FilterItems : public GenericObject
{
	GENERIC_OBJECT(FilterItems, allDataTypes)
public:
	PANDA_CLASS(FilterItems, GenericObject)

	FilterItems(PandaDocument *doc)
		: GenericObject(doc)
		, condition(initData(&condition, "condition", "Boolean values specifying which items are kept"))
		, generic(initData(&generic, "input", "Connect here the list"))
	{
		addInput(condition);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "The initial list of items"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Result of the deletion"));
		setupGenericObject(generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		const QVector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();
		const QVector<int> &valCond = condition.getValue();

		int nbIn = valIn.size(), nbCond = valCond.size();
		int size = std::min(nbIn, nbCond);
		valOut.clear();
		for(int i=0; i<size; ++i)
		{
			if(valCond[i])
				valOut << valIn[i];
		}
	}

protected:
	Data< QVector<int> > condition;
	GenericVectorData generic;
};

int FilterItemsClass = RegisterObject<FilterItems>("List/Filter items").setDescription("Remove items for which the condition is 0");

} // namespace Panda
