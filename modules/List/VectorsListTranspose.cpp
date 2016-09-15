#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>

namespace panda {

class VectorListTranspose : public GenericObject
{
public:
	PANDA_CLASS(VectorListTranspose, GenericObject)

		VectorListTranspose(PandaDocument *doc)
		: GenericObject(doc)
		, m_generic(initData("input", "Connect here the lists to transpose"))
	{
		addInput(m_generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "Input list");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Output,
							 "output",
							 "Output list");

		setupGenericObject<allListsVectorTypes>(this, m_generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		assert(dataInput && dataOutput);

		const auto& input = dataInput->getValue();
		auto acc = dataOutput->getAccessor();
		auto& output = acc.wref();
		output.clear();
		
		int maxSize = 0;
		for (const auto& list : input)
			maxSize = std::max<int>(maxSize, list.values.size());

		output.resize(maxSize);
		for (const auto& list : input)
		{
			int nb = list.values.size();
			for (int i = 0; i < maxSize; ++i)
				output[i].values.push_back((i < nb) ? list.values[i] : 0);
		}
	}

protected:
	GenericVectorData m_generic;
};

int VectorListTransposeClass = RegisterObject<VectorListTranspose>("List/Vectors list/Transpose lists")
	.setName("Vectors list transpose")
	.setDescription("Takes a number of lists and returns a list of corresponding size tuples (similar to a matrix transpose)");

} // namespace Panda
