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
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "list",
							 "List of items that will be added to the switch");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "result",
							 "Result of the switch");
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
		if (m_currentIndex == m_indexVal)
			outVal = dataInput->getValue();
		else if (!m_currentIndex)
			outVal.clear();

		++m_currentIndex;
	}

protected:
	Data<int> m_index;
	GenericVectorData generic;

	int m_indexVal = 0, m_currentIndex = 0;
};

int ListSwitchClass = RegisterObject<ListSwitch>("List/Switch whole")
		.setDescription("Choose one of multiple inputs");

//****************************************************************************//

class ListSwitchItem : public SingleTypeGenericObject
{
public:
	PANDA_CLASS(ListSwitchItem, SingleTypeGenericObject)

	ListSwitchItem(PandaDocument *doc)
		: SingleTypeGenericObject(doc)
		, m_index(initData("index", "Copy the input at this index to the output"))
		, generic(initData("input", "Connect here a list to add to the switch"))
	{
		m_singleOutput = true;

		addInput(m_index);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "list",
							 "List of items that will be added to the switch");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "result",
							 "Result of the switch");
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	void update()
	{
		m_indexVal = m_index.getValue();

		// First pass, we want to get the 
		m_listSizes.clear();
		m_getSizes = true;
		SingleTypeGenericObject::update();
		m_getSizes = false;

		if (m_indexVal.empty() || m_listSizes.empty())
			return;

		bool sizeError = false;
		m_outputSize = m_indexVal.size();
		for (auto size : m_listSizes)
		{
			if (size == 0)
				return;
			if (size == 1)
				continue;
			if (size > m_outputSize)
			{
				if (m_outputSize == 1 && !sizeError)
				{
					m_outputSize = size;
				}
				else
				{
					m_outputSize = 1;
					sizeError = true;
				}
			}
		}

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
		
		if (m_getSizes)
		{
			if (m_listSizes.empty())
				dataOutput->getAccessor().clear();

			m_listSizes.push_back(dataInput->getValue().size());
		}
		else
		{
			auto outVal = dataOutput->getAccessor();
			if (!m_currentIndex)
				outVal.resize(m_outputSize);

			const auto& input = dataInput->getValue();
			int nb = input.size();
			if (nb != m_outputSize)
				nb = 1;
			for (int i = 0; i < m_outputSize; ++i)
			{
				if (m_indexVal[i] == m_currentIndex)
					outVal[i] = input[i % nb];
			}

			++m_currentIndex;
		}
	}

protected:
	Data<std::vector<int>> m_index;
	GenericVectorData generic;

	std::vector<int> m_indexVal, m_listSizes;
	int m_currentIndex = 0, m_outputSize = 0;
	bool m_getSizes = false;
};

int ListSwitchItem_Class = RegisterObject<ListSwitchItem>("List/Switch items")
		.setDescription("Choose from multiple lists for each item");

} // namespace Panda
