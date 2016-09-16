#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/algorithm.h>
#include <panda/types/typesLists.h>

namespace panda {

class ListConcatenation : public SingleTypeGenericObject
{
public:
	PANDA_CLASS(ListConcatenation, SingleTypeGenericObject)

	ListConcatenation(PandaDocument *doc)
		: SingleTypeGenericObject(doc)
		, generic(initData("input", "Connect here a list to concatenate to the result"))
		, firstUpdate(false)
	{
		m_singleOutput = true;

		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "list",
							 "List of items that will be added to the result");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "result",
							 "Result of the concatenation");
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	void update()
	{
		firstUpdate = true;
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
		if(firstUpdate)
		{
			outVal.clear();
			firstUpdate = false;
		}
		const auto& inVal = dataInput->getValue();
		helper::concatenate(outVal.wref(), inVal);
	}

protected:
	GenericVectorData generic;
	bool firstUpdate;
};

int ListConcatenationClass = RegisterObject<ListConcatenation>("List/Concatenation")
		.setDescription("Concatenate multiple lists");

} // namespace Panda
