#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/GenericObject.h>

namespace panda {

class FillList  : public GenericObject
{
	GENERIC_OBJECT(FillList, allDataTypes)
public:
	PANDA_CLASS(FillList, GenericObject)

	FillList(PandaDocument *doc)
		: GenericObject(doc)
		, size(initData(&size, 10, "size", "Size of the list to be created"))
		, generic(initData(&generic, "input", "Connect here the lists to get the items from"))
	{
		addInput(&size);
		addInput(&generic);

		int typeOfList = DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "Value used to fill the list"));
		defList.append(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "List created from the given value"));

		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		int outputSize = size.getValue();
		typedef Data< QVector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		Q_ASSERT(dataInput && dataOutput);

		const QVector<T>& inVal = dataInput->getValue();
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

int FillListClass = RegisterObject("List/Fill list").setClass<FillList>().setDescription("Fill a list with a value");

} // namespace Panda
