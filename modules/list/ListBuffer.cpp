#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/GenericObject.h>

namespace panda {

class ListBuffer : public GenericObject
{
	GENERIC_OBJECT(ListBuffer, allDataTypes)

public:
	ListBuffer(PandaDocument *doc)
		: GenericObject(doc)
		, prevControl(-1.0)
		, control(initData(&control, 0.0, "control", "The buffer will be updated each time this value changes"))
		, generic(initData(&generic, "input", "Connect here the lists to put in the buffer"))
	{
		addInput(&control);
		addInput(&generic);

		int typeOfList = BaseData::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "Value to put in the buffer at each timestep"));
		defList.append(GenericDataDefinition(typeOfList,
											 true, false,
											 "init",
											 "Value to put in the buffer when the control is equal to zero"));
		defList.append(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Value stored in the buffer"));

		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		VecData *dataInput = dynamic_cast<VecData*>(list[0]);
		VecData *dataInit = dynamic_cast<VecData*>(list[1]);
		VecData *dataOutput = dynamic_cast<VecData*>(list[2]);
		QVector<T>& outVal = *(dataOutput->beginEdit());

		if(prevControl)
			outVal = dataInput->getValue();
		else
			outVal = dataInit->getValue();

		dataOutput->endEdit();
	}

	void update()
	{
		double newControl = control.getValue();
		if(prevControl != newControl)
		{
			prevControl = newControl;
			GenericObject::update();
		}
		this->cleanDirty();
	}

	void setDirtyValue()
	{
		if(prevControl != control.getValue())
			PandaObject::setDirtyValue();
	}

protected:
	double prevControl;
	Data<double> control;
	GenericVectorData generic;
};

int ListBufferClass = RegisterObject("List/Buffer").setClass<ListBuffer>().setName("Buffer").setDescription("Memorize a value and update it when the control value changes");

} // namespace Panda

