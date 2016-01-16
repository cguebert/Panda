#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>

namespace panda {

class InterleaveSequence : public SingleTypeGenericObject
{
public:
	PANDA_CLASS(InterleaveSequence, SingleTypeGenericObject)

	InterleaveSequence(PandaDocument *doc)
		: SingleTypeGenericObject(doc)
		, generic(initData("input", "Connect here a list to interleave to the result"))
		, index(0)
		, nbDatas(0)
	{
		m_singleOutput = true;

		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "list",
											 "List of items that will be interleaved to the result"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "result",
											 "Result of the interleaving"));
		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	void update()
	{
		index = 0;
		nbDatas = nbOfCreatedDatas();
		baseDataInputs.clear();
		SingleTypeGenericObject::update();
	}


	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		assert(dataInput && dataOutput);

		baseDataInputs.push_back(dataInput);

		index++;

		if(index == nbDatas)
		{
			std::vector<const std::vector<T>*> inputsList;
			for(auto baseDataInput : baseDataInputs)
				inputsList.push_back(&dynamic_cast<ListData*>(baseDataInput)->getValue());

			int nb = inputsList.size();
			int minSize = inputsList[0]->size();
			for(int i=0; i<nb; ++i)
				minSize = std::min<int>(minSize, inputsList[i]->size());

			auto outVal = dataOutput->getAccessor();
			outVal.wref().clear();
			outVal.wref().reserve(nb * minSize);

			for(int i=0; i<minSize; ++i)
			{
				for(int j=0; j<nb; ++j)
					outVal.push_back( (*inputsList[j])[i] );
			}
		}
	}

protected:
	GenericVectorData generic;
	std::vector<BaseData*> baseDataInputs;
	int index, nbDatas;
};

int InterleaveSequenceClass = RegisterObject<InterleaveSequence>("List/Interleave sequence")
		.setDescription("Merge multiples lists by interleaving values of each list");

} // namespace Panda
