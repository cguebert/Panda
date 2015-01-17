#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

using types::DataTypeId;

class FindItem : public GenericObject
{
	GENERIC_OBJECT(FindItem, allSearchableTypes)
public:
	PANDA_CLASS(FindItem, GenericObject)

	FindItem(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the lists in which to look for the values"))
	{
		addInput(&generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "List in which to look for the values"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "values",
											 "List of values to look for"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
											 false, true,
											 "output",
											 "Indices of the values in the input list"));

		setupGenericObject(generic, defList);
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
		id.clear();

		int size = inList.size();
		if(size)
		{
			int nbVal = inVal.size();
			for(int i=0; i<size; ++i)
			{
				const T& listItem = inList[i];
				for(int j=0; j<nbVal; ++j)
				{
					if(listItem == inVal[j])
					{
						id.push_back(i);
						break;
					}
				}
			}
		}
	}

protected:
	GenericVectorData generic;
};

int FindItemClass = RegisterObject<FindItem>("List/Find item").setDescription("Find some values in a list");

} // namespace Panda
