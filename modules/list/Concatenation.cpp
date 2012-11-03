#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListConcatenation : public GenericObject
{
	GENERIC_OBJECT(ListConcatenation, allDataTypes)
public:
	ListConcatenation(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the first list"))
    {
        addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = BaseData::getFullTypeOfVector(0);
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "head",
											 "List of items that will be at the head of the result"));
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "tail",
											 "List of items that will be at the tail of the result"));
		defList.append(GenericDataDefinition(listType,
											 false, true,
											 "result",
											 "Result of the concatenation"));
		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataHead = dynamic_cast<ListData*>(list[0]);
		ListData* dataTail = dynamic_cast<ListData*>(list[1]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[2]);

		Q_ASSERT(dataHead && dataTail && dataOutput);

		QVector<T>& outVal = *dataOutput->beginEdit();
		outVal.clear();
		outVal << dataHead->getValue() << dataTail->getValue();
		dataOutput->endEdit();
	}

protected:
	GenericVectorData generic;
};

int ListConcatenationClass = RegisterObject("List/Concatenation").setClass<ListConcatenation>().setDescription("Concatenate two lists");

} // namespace Panda
