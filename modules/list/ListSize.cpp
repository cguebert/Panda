#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListSize : public GenericObject
{
	GENERIC_OBJECT(ListSize, allDataTypes)
public:
	PANDA_CLASS(ListSize, GenericObject)

	ListSize(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the lists to analyse"))
    {
        addInput(&generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfVector(0),
											 true, false,
											 "input",
											 "List to analyse"));
		// Create a single int value
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfSingleValue(BaseData::getValueTypeOf<int>()),
											 false, true,
											 "size",
											 "Number of values in this list"));
		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		typedef Data< int > IntData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		IntData* dataSize = dynamic_cast<IntData*>(list[1]);

		Q_ASSERT(dataInput && dataSize);

		const QVector<T>& inVal = dataInput->getValue();
		dataSize->setValue(inVal.size());
	}

protected:
	GenericVectorData generic;
};

int ListSizeClass = RegisterObject("List/List size").setClass<ListSize>().setDescription("Get the size of some lists");

} // namespace Panda
