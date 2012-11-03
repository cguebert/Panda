#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

namespace panda {

class ExtractTail : public GenericObject
{
	GENERIC_OBJECT(ExtractTail, allDataTypes)
public:
	ExtractTail(PandaDocument *doc)
		: GenericObject(doc)
		, value(initData(&value, 1, "tail", "All items starting at this index will be extracted. If negative, extract this number of items instead."))
		, generic(initData(&generic, "input", "Connect here the first list"))
	{
		addInput(&value);
		addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = BaseData::getFullTypeOfVector(0);
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List of items from which to extract the tail"));
		defList.append(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Result of the extraction"));
		setupGenericObject(&generic, defList);
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
		QVector<T> &valOut = *dataOutput->beginEdit();

		if(val > 0)
			valOut.swap( valIn.mid(val) );
		else
			valOut.swap( valIn.mid(valIn.size() + val) );

		dataOutput->endEdit();
	}

protected:
	Data< int > value;
	GenericVectorData generic;
};

int ExtractTailClass = RegisterObject("List/Get tail").setClass<ExtractTail>().setDescription("Get the last items of a list");

} // namespace Panda
