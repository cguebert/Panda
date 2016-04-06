#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

class ListCondition : public GenericObject
{
public:
	PANDA_CLASS(ListCondition, GenericObject)

	ListCondition(PandaDocument *doc)
		: GenericObject(doc)
		, control(initData("control", "If this value is non zero copy the first list, otherwise copy the second"))
		, generic(initData("input", "Connect here the first list"))
	{
		addInput(control);
		addInput(generic);

		control.getAccessor().push_back(1);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "true",
											 "List of items copied if control is true"));
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "false",
											 "List of items copied if control is false"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "result",
											 "Result of the condition"));
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInTrue = dynamic_cast<ListData*>(list[0]);
		ListData* dataInFalse = dynamic_cast<ListData*>(list[1]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[2]);

		assert(dataInTrue && dataInFalse && dataOutput);

		const auto& c = control.getValue();
		int nbC = c.size();
		auto outVal = dataOutput->getAccessor();
		if(!nbC)
		{
			outVal.clear();
			return;
		}

		const auto& inT = dataInTrue->getValue();
		const auto& inF = dataInFalse->getValue();
		int nb = std::min(inT.size(), inF.size());
		if(nbC == 1 || nbC < nb)
			outVal = c[0] ? inT : inF;
		else if(nb == 1)
		{
			outVal.resize(nbC);
			for(int i=0; i<nbC; ++i)
				outVal[i] = c[i] ? inT[0] : inF[0];
		}
		else // nbC == nb
		{
			outVal.resize(nb);
			for(int i=0; i<nb; ++i)
				outVal[i] = c[i] ? inT[i] : inF[i];
		}
	}

protected:
	Data< std::vector<int> > control;
	GenericVectorData generic;
};

int ListConditionClass = RegisterObject<ListCondition>("List/Condition").setDescription("Copy one of two lists based on the condition value");

} // namespace Panda
