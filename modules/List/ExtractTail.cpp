#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ExtractTail : public GenericObject
{
	GENERIC_OBJECT(ExtractTail, allDataTypes)
public:
	PANDA_CLASS(ExtractTail, GenericObject)

	ExtractTail(PandaDocument *doc)
		: GenericObject(doc)
		, value(initData(&value, 1, "tail", "All items starting at this index will be extracted. If negative, extract this number of items instead."))
		, generic(initData(&generic, "input", "Connect here the first list"))
	{
		addInput(value);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List of items from which to extract the tail"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Result of the extraction"));
		setupGenericObject(generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		int val = value.getValue();
		const QVector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();
		int nb = valIn.size();

		if(val >= nb)
			valOut.clear();
		else if(val > 0)
			valOut = valIn.mid(val);
		else
			valOut = valIn.mid( qMax(0, valIn.size() + val) );
	}

protected:
	Data< int > value;
	GenericVectorData generic;
};

int ExtractTailClass = RegisterObject<ExtractTail>("List/Get tail").setDescription("Get the last items of a list");

} // namespace Panda
