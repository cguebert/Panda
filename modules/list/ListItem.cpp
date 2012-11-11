#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListItem : public GenericObject
{
	GENERIC_OBJECT(ListItem, allDataTypes)
public:
    ListItem(PandaDocument *doc)
		: GenericObject(doc)
        , indexData(initData(&indexData, "index", "0-based index of the items to extract from the lists"))
        , generic(initData(&generic, "input", "Connect here the lists to get the items from"))
    {
        addInput(&indexData);
        addInput(&generic);

        indexData.beginEdit()->append(0);
        indexData.endEdit();

		int typeOfList = BaseData::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "List from which to extract values"));
		defList.append(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Value extracted from the list"));

		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		Q_ASSERT(dataInput && dataOutput);

		const QVector<int>& id = indexData.getValue();
		const QVector<T>& inVal = dataInput->getValue();
		QVector<T>& outVal = *(dataOutput->beginEdit());

		int size = inVal.size();
		int nb = id.size();
		outVal.resize(nb);
		for(int i=0; i<nb; ++i)
			outVal[i] = inVal[qBound(0, id[i], size-1)];

		dataOutput->endEdit();
	}

protected:
    Data< QVector<int> > indexData;
    GenericVectorData generic;
};

int ListItemClass = RegisterObject("List/List item").setClass<ListItem>().setDescription("Extract an item from a list");

} // namespace Panda
