#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>

namespace panda {

using types::DataTypeId;

class ListSize : public GenericObject
{
	GENERIC_OBJECT(ListSize, allDataTypes)
public:
	PANDA_CLASS(ListSize, GenericObject)

	ListSize(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to analyse"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input",
											 "List to analyse"));
		// Create a single int value
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
											 false, true,
											 "size",
											 "Number of values in this list"));
		setupGenericObject(generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		typedef Data< int > IntData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		IntData* dataSize = dynamic_cast<IntData*>(list[1]);

		Q_ASSERT(dataInput && dataSize);

		const std::vector<T>& inVal = dataInput->getValue();
		dataSize->setValue(inVal.size());
	}

protected:
	GenericVectorData generic;
};

int ListSizeClass = RegisterObject<ListSize>("List/List size").setDescription("Get the size of some lists");

} // namespace Panda
