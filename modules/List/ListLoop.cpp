#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>
#include <panda/types/typesLists.h>

#include <iostream>

namespace panda {

class ListLoop : public GenericObject
{
public:
	PANDA_CLASS(ListLoop, GenericObject)

	ListLoop(PandaDocument *doc)
		: GenericObject(doc)
		, m_copyInit(true)
		, prevControl(-1.0)
		, m_control(initData((float)0.0, "control", "This object will be updated each time this value changes"))
		, m_condition(initData(1, "condition", "Continue while this condition is true (non zero)"))
		, m_nbIterations(initData(5, "nbIterations", "Maximum number of iterations"))
		, m_iteration(initData(0, "iteration", "Current iteration"))
		, m_generic(initData("input", "Connect here the lists to use in the loop"))
	{
		addInput(m_control);
		addInput(m_condition);
		addInput(m_nbIterations);
		addInput(m_generic);
		addOutput(m_iteration);

		m_condition.setWidget("checkbox");

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "Value to use for the next iteration");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "init",
							 "Value before the first iteration");
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Output,
							 "output",
							 "Corresponding value at the previous iteration");

		setupGenericObject<allDataTypes>(this, m_generic, defList);

		setLaterUpdate(true);
	}

	NodesList getNonRecursiveInputs() const override
	{
		// Convert from const Data* to DataNode*
		return { (DataNode*)&m_control, (DataNode*)&m_condition, (DataNode*)&m_nbIterations };
	}

	NodesList getNonRecursiveOutputs() const override
	{
		return { (DataNode*)&m_iteration };
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataInit = dynamic_cast<VecData*>(list[1]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[2]);
		assert(dataInput && dataInit && dataOutput);

		parentDocument()->setDataDirty(dataOutput);
		if(m_copyInit)
			dataOutput->getAccessor() = dataInit->getValue();
		else
			dataOutput->getAccessor() = dataInput->getValue();
		parentDocument()->setDataReady(dataOutput);
	}

	void update()
	{
		float newControl = m_control.getValue();
		if(prevControl != newControl)
		{
			prevControl = newControl;

			m_copyInit = true;
			GenericObject::doUpdate(false); // Do not update the Datas we don't use
			m_copyInit = false;

			for(int i=0, nb = m_nbIterations.getValue(); i < nb; ++i)
			{
				m_iteration.setValue(i);
				if(!m_condition.getValue())
					break;
				GenericObject::doUpdate(false); // Do not update the Datas we don't use
			}
		}
	}

	void setDirtyValue(const DataNode* caller)
	{
		if (isDestructing() || caller == &m_condition)
			return;

		if(!isDirty())
		{
			if(caller == &m_control || caller == this)
				PandaObject::setDirtyValue(caller);
		}
	}

	void reset() override
	{
		prevControl = FLT_MAX;
		update();
	}

protected:
	bool m_copyInit = false;
	float prevControl;
	Data<float> m_control;
	Data<int> m_condition, m_nbIterations, m_iteration;
	GenericVectorData m_generic;
};

int ListLoopClass = RegisterObject<ListLoop>("List/Loop").setDescription("Update values multiple times");

} // namespace Panda

