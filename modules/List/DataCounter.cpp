#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>

namespace panda {

class ListDataCounter : public GenericObject
{
public:
	PANDA_CLASS(ListDataCounter, GenericObject)

	ListDataCounter(PandaDocument *doc)
		: GenericObject(doc)
		, m_generic(initData("input", "Connect here the lists to put in the buffer"))
	{
		addInput(m_generic);

		int intTypeId = types::DataTypeId::getFullTypeOfSingleValue(types::DataTypeId::getIdOf<int>());
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(0, // use the exact type of the input data
											 true, false,
											 "data",
											 "Data of which to show the counter"));
		defList.push_back(GenericDataDefinition(intTypeId,
											 false, true,
											 "counter",
											 "Counter of the input data"));

		setupGenericObject<allDataTypes>(this, m_generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data<int> IntData;
		BaseData* dataInput = list[0];
		IntData* dataCounter = dynamic_cast<IntData*>(list[1]);
		assert(dataInput && dataCounter);

		dataCounter->setValue(dataInput->getCounter());
	}

protected:
	GenericData m_generic;
};

int ListDataCounterClass = RegisterObject<ListDataCounter>("List/Data counter").setDescription("Show the data counter that increments each time its value changes");

} // namespace Panda

