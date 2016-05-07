#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

namespace panda {

class ListBuffer : public GenericObject
{
public:
	PANDA_CLASS(ListBuffer, GenericObject)

	ListBuffer(PandaDocument *doc)
		: GenericObject(doc)
		, resetValues(true)
		, prevControl(-1.0)
		, control(initData((float)0.0, "control", "The buffer will be updated each time this value changes"))
		, resetData(initData(0, "reset", "Set this at 1 to reset the values"))
		, generic(initData("input", "Connect here the lists to put in the buffer"))
	{
		addInput(control);
		addInput(resetData);
		addInput(generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "Value to put in the buffer at each timestep");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "init",
							 "Value to put in the buffer when reset is non null");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Output,
							 "output",
							 "Value stored in the buffer");

		setupGenericObject<allDataTypes>(this, generic, defList);

		setLaterUpdate(true);
	}

	NodesList getNonRecursiveInputs() const override
	{
		// Convert from const Data* to DataNode*
		return { (DataNode*)&control, (DataNode*)&resetData };
	}

	NodesList getNonRecursiveOutputs() const override
	{
		return { };
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataInit = dynamic_cast<VecData*>(list[1]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[2]);
		assert(dataInput && dataInit && dataOutput);

		const std::vector<T>& value = resetValues ? dataInit->getValue() : dataInput->getValue();
		parentDocument()->setDataDirty(dataOutput);
		dataOutput->getAccessor() = value;
		parentDocument()->setDataReady(dataOutput);
	}

	void update()
	{
		resetValues = resetValues || (resetData.getValue() != 0);	// Either from the data, or if the reset function was called
		float newControl = control.getValue();
		if(prevControl != newControl || resetValues)
		{
			prevControl = newControl;
			GenericObject::update();
		}
		resetValues = false;
	}

	void reset()
	{
		PandaObject::reset();

		resetValues = true;
		setDirtyValue(this);
	}

	void setDirtyValue(const DataNode* caller)
	{
		if(!isDirty())
		{
			if(caller == &control || caller == &resetData || caller == this)
				PandaObject::setDirtyValue(caller);
		}
	}

protected:
	bool resetValues;
	float prevControl;
	Data<float> control;
	Data<int> resetData;
	GenericVectorData generic;
};

int ListBufferClass = RegisterObject<ListBuffer>("List/Buffer").setDescription("Memorize a value and update it when the control value changes");

} // namespace Panda

