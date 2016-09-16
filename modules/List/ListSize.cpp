#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

using types::DataTypeId;

class ListSize : public GenericObject
{
public:
	PANDA_CLASS(ListSize, GenericObject)

	ListSize(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to analyse"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List to analyse");
		// Create a single int value
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "size",
							 "Number of values in this list");
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		typedef Data< int > IntData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		IntData* dataSize = dynamic_cast<IntData*>(list[1]);

		assert(dataInput && dataSize);

		const std::vector<T>& inVal = dataInput->getValue();
		dataSize->setValue(inVal.size());
	}

protected:
	GenericVectorData generic;
};

int ListSizeClass = RegisterObject<ListSize>("List/List size").setDescription("Get the size of some lists");

//****************************************************************************//

class VectorListSize : public GenericObject
{
public:
	PANDA_CLASS(VectorListSize, GenericObject)

		VectorListSize(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the lists to analyse"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List to analyse");
		// Create a single int value
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "size",
							 "Number of values in each vector");
		setupGenericObject<allListsVectorTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		typedef Data< std::vector<int> > IntData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		IntData* dataSize = dynamic_cast<IntData*>(list[1]);

		assert(dataInput && dataSize);

		const std::vector<T>& inVal = dataInput->getValue();
		int nb = inVal.size();
		std::vector<int> sizes(nb);
		for (int i = 0; i < nb; ++i)
			sizes[i] = inVal[i].values.size();
		dataSize->setValue(sizes);
	}

protected:
	GenericVectorData generic;
};

int VectorListSizeClass = RegisterObject<VectorListSize>("List/Vectors list/Lists size")
	.setName("Vectors list size")
	.setDescription("Get the size of some lists of vectors");

} // namespace Panda
