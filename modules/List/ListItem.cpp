#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListItem : public GenericObject
{
	GENERIC_OBJECT(ListItem, allDataTypes)
public:
	PANDA_CLASS(ListItem, GenericObject)

	ListItem(PandaDocument *doc)
		: GenericObject(doc)
		, indexData(initData("index", "0-based index of the items to extract from the lists"))
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(indexData);
		addInput(generic);

		indexData.getAccessor().push_back(0);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "List from which to extract values"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Value extracted from the list"));

		setupGenericObject(generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		Q_ASSERT(dataInput && dataOutput);

		const std::vector<int>& id = indexData.getValue();
		const std::vector<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		int size = inVal.size();
		int nb = id.size();
		outVal.resize(nb);
		if(size)
		{
			for(int i=0; i<nb; ++i)
				outVal[i] = inVal[qBound(0, id[i], size-1)];
		}
	}

protected:
	Data< std::vector<int> > indexData;
	GenericVectorData generic;
};

int ListItemClass = RegisterObject<ListItem>("List/List item").setDescription("Extract an item from a list");

} // namespace Panda
