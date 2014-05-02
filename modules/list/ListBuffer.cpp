#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/GenericObject.h>

namespace panda {

class ListBuffer : public GenericObject
{
	GENERIC_OBJECT(ListBuffer, allDataTypes)

public:
	PANDA_CLASS(ListBuffer, GenericObject)

	ListBuffer(PandaDocument *doc)
		: GenericObject(doc)
		, resetValues(false)
		, settingDirty(false)
		, prevControl(-1.0)
		, control(initData(&control, (PReal)0.0, "control", "The buffer will be updated each time this value changes"))
		, resetData(initData(&resetData, 0, "reset", "Set this at 1 to reset the values"))
		, generic(initData(&generic, "input", "Connect here the lists to put in the buffer"))
	{
		addInput(&control);
		addInput(&resetData);
		addInput(&generic);

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "Value to put in the buffer at each timestep"));
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "init",
											 "Value to put in the buffer when reset is non null"));
		defList.append(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Value stored in the buffer"));

		setupGenericObject(&generic, defList);

		laterUpdate = true;
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataInit = dynamic_cast<VecData*>(list[1]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[2]);
		Q_ASSERT(dataInput && dataInit && dataOutput);

		const QVector<T>& value = resetValues ? dataInit->getValue() : dataInput->getValue();
		parentDocument->setDataDirty(dataOutput);
		dataOutput->getAccessor() = value;
		parentDocument->setDataReady(dataOutput);
	}

	void update()
	{
		resetValues = resetValues || (resetData.getValue() != 0);	// Either from the data, or if the reset function was called
		PReal newControl = control.getValue();
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
		settingDirty = true;
		PandaObject::setDirtyValue();
		update();
		settingDirty = false;
	}

	void setDirtyValue()
	{
		if(!settingDirty)
		{
			PReal newControl = control.getValue();
			if(prevControl != newControl || resetValues || (resetData.getValue() != 0))
			{
				settingDirty = true;
				PandaObject::setDirtyValue();
				update();
				settingDirty = false;
			}
		}
	}

protected:
	bool resetValues, settingDirty;
	PReal prevControl;
	Data<PReal> control;
	Data<int> resetData;
	GenericVectorData generic;
};

int ListBufferClass = RegisterObject<ListBuffer>("List/Buffer").setDescription("Memorize a value and update it when the control value changes");

} // namespace Panda

