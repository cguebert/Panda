#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListCondition : public GenericObject
{
	GENERIC_OBJECT(ListCondition, allDataTypes)
public:
	PANDA_CLASS(ListCondition, GenericObject)

	ListCondition(PandaDocument *doc)
		: GenericObject(doc)
		, control(initData(&control, 1, "control", "If this value is non zero copy the first list, otherwise copy the second"))
		, generic(initData(&generic, "input", "Connect here the first list"))
	{
		addInput(&control);
		addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = DataTypeId::getFullTypeOfVector(0);
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "true",
											 "List of items copied if control is true"));
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "false",
											 "List of items copied if control is false"));
		defList.append(GenericDataDefinition(listType,
											 false, true,
											 "result",
											 "Result of the condition"));
		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInTrue = dynamic_cast<ListData*>(list[0]);
		ListData* dataInFalse = dynamic_cast<ListData*>(list[1]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[2]);

		Q_ASSERT(dataInTrue && dataInFalse && dataOutput);

		auto outVal = dataOutput->getAccessor();
		if(control.getValue())
			outVal = dataInTrue->getValue();
		else
			outVal = dataInFalse->getValue();
	}

protected:
	Data<int> control;
	GenericVectorData generic;
};

int ListConditionClass = RegisterObject("List/Condition").setClass<ListCondition>().setDescription("Copy one of two lists based on the condition value");

} // namespace Panda
