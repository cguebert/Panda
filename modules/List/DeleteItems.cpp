#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

class DeleteItems : public GenericObject
{
public:
	PANDA_CLASS(DeleteItems, GenericObject)

	DeleteItems(PandaDocument *doc)
		: GenericObject(doc)
		, indices(initData("indices", "Indices of items to be removed from the list"))
		, generic(initData("input", "Connect here the list"))
	{
		addInput(indices);
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.emplace_back(listType,
							 GenericDataDefinition::Input,
							 "input",
							 "The initial list of items");
		defList.emplace_back(listType,
							 GenericDataDefinition::Output,
							 "output",
							 "Result of the deletion");
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		assert(dataInput && dataOutput);

		const std::vector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();
		const std::vector<int> &valId = indices.getValue();

		std::vector<bool> mask;
		unsigned int nbIn = valIn.size(), nbId = valId.size();
		mask.resize(nbIn, true);
		for(unsigned int i=0; i<nbId; ++i)
		{
			unsigned int id = valId[i];
			if(id < nbIn)
				mask[id] = false;	// Mark for deletion
		}

		valOut.clear();
		for(unsigned int i=0; i<nbIn; ++i)
		{
			if(mask[i])
				valOut.push_back(valIn[i]);
		}

	}

protected:
	Data< std::vector<int> > indices;
	GenericVectorData generic;
};

int DeleteItemsClass = RegisterObject<DeleteItems>("List/Delete items").setDescription("Remove some items from a list");

} // namespace Panda
