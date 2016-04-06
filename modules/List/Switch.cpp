#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/algorithm.h>
#include <panda/types/typesLists.h>

namespace panda {

class ListSwitch : public SingleTypeGenericObject
{
public:
	PANDA_CLASS(ListSwitch, SingleTypeGenericObject)

	ListSwitch(PandaDocument *doc)
		: SingleTypeGenericObject(doc)
		, m_index(initData(0, "index", "Copy the input at this index to the output"))
		, generic(initData("input", "Connect here a list to add to the switch"))
	{
		m_singleOutput = true;

		addInput(m_index);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "list",
											 "List of items that will be added to the switch"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "result",
											 "Result of the switch"));
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	void update()
	{
		m_indexVal = m_index.getValue();
		m_currentIndex = 0;
		SingleTypeGenericObject::update();
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		assert(dataInput && dataOutput);

		auto outVal = dataOutput->getAccessor();
		if (!m_currentIndex)
			outVal.clear();

		if (m_currentIndex++ != m_indexVal)
			return;

		outVal = dataInput->getValue();
	}

protected:
	Data<int> m_index;
	GenericVectorData generic;

	int m_indexVal = 0, m_currentIndex = 0;
};

int ListSwitchClass = RegisterObject<ListSwitch>("List/Switch")
		.setDescription("Choose one of multiple inputs");

} // namespace Panda
