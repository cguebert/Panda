#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

namespace panda {

class DeleteItems : public GenericObject
{
	GENERIC_OBJECT(DeleteItems, allDataTypes)
public:
	PANDA_CLASS(DeleteItems, GenericObject)

	DeleteItems(PandaDocument *doc)
		: GenericObject(doc)
		, indices(initData(&indices, "indices", "Indices of items to be removed from the list"))
		, generic(initData(&generic, "input", "Connect here the list"))
    {
		addInput(&indices);
        addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = BaseData::getFullTypeOfVector(0);
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "The initial list of items"));
		defList.append(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Result of the deletion"));
		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		const QVector<T> &valIn = dataInput->getValue();
		QVector<T> &valOut = *dataOutput->beginEdit();
		const QVector<int> &valId = indices.getValue();

		QVector<bool> mask;
		unsigned int nbIn = valIn.size(), nbId = valId.size();
		mask.fill(true, nbIn);
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
				valOut << valIn[i];
		}

		dataOutput->endEdit();
	}

protected:
	Data< QVector<int> > indices;
	GenericVectorData generic;
};

int DeleteItemsClass = RegisterObject("List/Delete items").setClass<DeleteItems>().setDescription("Remove some items from a list");

} // namespace Panda
