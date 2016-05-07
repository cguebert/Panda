#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

using types::DataTypeId;

class FindItem : public GenericObject
{
public:
	PANDA_CLASS(FindItem, GenericObject)

	FindItem(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists in which to look for the values"))
	{
		addInput(generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "List in which to look for the values");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "values",
							 "List of values to look for");
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "output",
							 "Indices of the values in the input list");

		setupGenericObject<allSearchableTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		typedef Data< std::vector<int> > VecIntData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataValues = dynamic_cast<VecData*>(list[1]);
		VecIntData* dataOutput = dynamic_cast<VecIntData*>(list[2]);
		assert(dataInput && dataValues && dataOutput);

		const std::vector<T>& inList = dataInput->getValue();
		const std::vector<T>& inVal = dataValues->getValue();
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
