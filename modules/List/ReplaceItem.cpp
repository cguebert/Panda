#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>

namespace panda {

using types::DataTypeId;

class ReplaceItem : public GenericObject
{
public:
	PANDA_CLASS(ReplaceItem, GenericObject)

	ReplaceItem(PandaDocument *doc)
		: GenericObject(doc)
		, indices(initData("indices", "Indices of the items to replace"))
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(indices);
		addInput(generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "Initial list in which to replace some values"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "values",
											 "Values to insert into the list"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Modified list"));

		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		typedef Data< std::vector<int> > VecIntData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataValues = dynamic_cast<VecData*>(list[1]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[2]);
		assert(dataInput && dataValues && dataOutput);

		const std::vector<T>& inList = dataInput->getValue();
		const std::vector<int>& id = indices.getValue();
		const std::vector<T>& inVal = dataValues->getValue();
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
	Data< std::vector<int> > indices;
	GenericVectorData generic;
};

int ReplaceItemClass = RegisterObject<ReplaceItem>("List/Replace item").setDescription("Replace some items in a list by new values");

} // namespace Panda
