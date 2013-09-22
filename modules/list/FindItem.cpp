#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class FindItem : public GenericObject
{
	GENERIC_OBJECT(FindItem, allDataTypes)
public:
	PANDA_CLASS(FindItem, GenericObject)

	FindItem(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the lists in which to look for the values"))
    {
        addInput(&generic);

		int typeOfList = BaseData::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "List in which to look for the values"));
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "values",
											 "List of values to look for"));
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfVector(BaseData::getValueTypeOf<int>()),
											 false, true,
											 "output",
											 "Indices of the values in the input list"));

		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		typedef Data< QVector<int> > VecIntData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataValues = dynamic_cast<VecData*>(list[1]);
		VecIntData* dataOutput = dynamic_cast<VecIntData*>(list[2]);
		Q_ASSERT(dataInput && dataValues && dataOutput);

		const QVector<T>& inList = dataInput->getValue();
		const QVector<T>& inVal = dataValues->getValue();
		auto id = dataOutput->getAccessor();

		int size = inList.size();
		if(size)
		{
			int nb = inVal.size();
			id.resize(nb);
			for(int i=0; i<nb; ++i)
				id[i] = inList.indexOf(inVal[i]);
		}
		else
			id.clear();
	}

protected:
    GenericVectorData generic;
};

int FindItemClass = RegisterObject("List/Find item").setClass<FindItem>().setDescription("Find some values in a list");

} // namespace Panda
