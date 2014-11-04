#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

using types::DataTypeId;

class ReplaceItem : public GenericObject
{
	GENERIC_OBJECT(ReplaceItem, allDataTypes)
public:
	PANDA_CLASS(ReplaceItem, GenericObject)

	ReplaceItem(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the lists to get the items from"))
	{
		addInput(&generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "Initial list in which to replace some values"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
											 true, false,
											 "indices",
											 "Indices of the items to replace"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "values",
											 "Values to insert into the list"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Modified list"));

		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		typedef Data< QVector<int> > VecIntData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecIntData* dataIndices = dynamic_cast<VecIntData*>(list[1]);
		VecData* dataValues = dynamic_cast<VecData*>(list[2]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[3]);
		Q_ASSERT(dataInput && dataIndices && dataValues && dataOutput);

		const QVector<T>& inList = dataInput->getValue();
		const QVector<int>& id = dataIndices->getValue();
		const QVector<T>& inVal = dataValues->getValue();
		auto outVal = dataOutput->getAccessor();

		int listSize = inList.size();
		int idSize = id.size();
		int valSize = inVal.size();

		outVal = inList;
		if(listSize && idSize && valSize && idSize == valSize)
		{
			for(int i=0; i<idSize; ++i)
			{
				if(id[i] >= 0 && id[i] < listSize)
					outVal[id[i]] = inVal[i];
			}
		}
	}

protected:
	GenericVectorData generic;
};

int ReplaceItemClass = RegisterObject<ReplaceItem>("List/Replace item").setDescription("Replace some items in a list by new values");

} // namespace Panda