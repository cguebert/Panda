#include <panda/document/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

namespace panda {

class FillList  : public GenericObject
{
public:
	PANDA_CLASS(FillList, GenericObject)

	FillList(PandaDocument *doc)
		: GenericObject(doc)
		, size(initData(10, "size", "Size of the list to be created"))
		, generic(initData("input", "Connect here the lists to get the items from"))
	{
		addInput(size);
		addInput(generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "Value used to fill the list");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Output,
							 "output",
							 "List created from the given value");

		setupGenericObject<allDataTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		int outputSize = size.getValue();
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		assert(dataInput && dataOutput);

		const std::vector<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();
		int inputSize = inVal.size();

		if(outputSize && inputSize)
		{
			outVal.resize(outputSize);

			for(int i=0; i<outputSize; ++i)
				outVal[i] = inVal[i % inputSize];
		}
		else
			outVal.clear();
	}


protected:
	Data<int> size;
	GenericVectorData generic;
};

int FillListClass = RegisterObject<FillList>("List/Fill list").setDescription("Fill a list with a value");

} // namespace Panda
