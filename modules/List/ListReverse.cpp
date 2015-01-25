#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ListReverse : public GenericObject
{
	GENERIC_OBJECT(ListReverse, allDataTypes)
public:
	PANDA_CLASS(ListReverse, GenericObject)

	ListReverse(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the list whose items to reverse"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List to reverse"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Reversed list"));
		setupGenericObject(generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		const QVector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();

		int size = valIn.size();
		valOut.resize(size);
		for(int i=0; i<size; ++i)
			valOut[i] = valIn[size-1-i];
	}

protected:
	GenericVectorData generic;
};

int ListReverseClass = RegisterObject<ListReverse>("List/Reverse").setDescription("Reverse the order of items in a list");

} // namespace Panda
