#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

namespace panda {

class ListDataCounter : public GenericObject
{
public:
	PANDA_CLASS(ListDataCounter, GenericObject)

	ListDataCounter(PandaDocument *doc)
		: GenericObject(doc)
		, m_generic(initData("input", "Connect here the datas from which to get the counter"))
	{
		addInput(m_generic);

		int intTypeId = types::DataTypeId::getFullTypeOfSingleValue(types::DataTypeId::getIdOf<int>());
		GenericDataDefinitionList defList;
		defList.emplace_back(0, // use the exact type of the input data
							 GenericDataDefinition::Input,
							 "data",
							 "Data of which to show the counter");
		defList.emplace_back(intTypeId,
							 GenericDataDefinition::Output,
							 "counter",
							 "Counter of the input data");

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

//****************************************************************************//

class ListDataCounterSum : public GenericObject
{
public:
	PANDA_CLASS(ListDataCounterSum, GenericObject)

	ListDataCounterSum(PandaDocument *doc)
		: GenericObject(doc)
		, m_counter(initData(0, "counter", "Sum of the data counters of the inputs"))
		, m_generic(initData("input", "Connect here the datas from which to get the counter"))
	{
		addInput(m_generic);
		addOutput(m_counter);

		GenericDataDefinitionList defList;
		defList.emplace_back(0, // use the exact type of the input data
							 GenericDataDefinition::Input,
							 "data",
							 "Data of which to show the counter");

		setupGenericObject<allDataTypes>(this, m_generic, defList);
	}

	void update()
	{
		m_tempSum = 0;
		GenericObject::doUpdate();

		m_counter.setValue(m_tempSum);
	}

	template <class T>
	void updateT(DataList& list)
	{
		BaseData* dataInput = list[0];
		assert(dataInput);

		m_tempSum += dataInput->getCounter();
	}

protected:
	Data<int> m_counter;
	GenericData m_generic;

	int m_tempSum = 0;
};

int ListDataCounterSumClass = RegisterObject<ListDataCounterSum>("List/Data counters sum").setDescription("Compute the sum of the data counters of all inputs");

} // namespace Panda

