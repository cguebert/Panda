#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListConcatenation : public SingleTypeGenericObject
{
	GENERIC_OBJECT(ListConcatenation, allDataTypes)
public:
	PANDA_CLASS(ListConcatenation, SingleTypeGenericObject)

	ListConcatenation(PandaDocument *doc)
		: SingleTypeGenericObject(doc)
		, generic(initData(&generic, "input", "Connect here a list to concatenate to the result"))
		, firstUpdate(false)
	{
		m_singleOutput = true;

		addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "list",
											 "List of items that will be added to the result"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "result",
											 "Result of the concatenation"));
		setupGenericObject(&generic, defList);
	}

	void update()
	{
		firstUpdate = true;
		SingleTypeGenericObject::update();
	}


	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		auto outVal = dataOutput->getAccessor();
		if(firstUpdate)
		{
			outVal.clear();
			firstUpdate = false;
		}
		outVal.wref() += dataInput->getValue();
	}

protected:
	GenericVectorData generic;
	bool firstUpdate;
};

int ListConcatenationClass = RegisterObject<ListConcatenation>("List/Concatenation")
		.setDescription("Concatenate multiple lists");

} // namespace Panda