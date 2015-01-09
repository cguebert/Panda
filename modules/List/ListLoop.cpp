#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/GenericObject.h>

#include <iostream>

namespace panda {

class ListLoop : public GenericObject
{
	GENERIC_OBJECT(ListLoop, allDataTypes)

public:
	PANDA_CLASS(ListLoop, GenericObject)

	ListLoop(PandaDocument *doc)
		: GenericObject(doc)
		, m_copyInit(true)
		, prevControl(-1.0)
		, m_control(initData(&m_control, (PReal)0.0, "control", "This object will be updated each time this value changes"))
		, m_condition(initData(&m_condition, 1, "condition", "Continue while this condition is true (non zero)"))
		, m_nbIterations(initData(&m_nbIterations, 5, "nbIterations", "Maximum number of iterations"))
		, m_iteration(initData(&m_iteration, 0, "iteration", "Current iteration"))
		, m_generic(initData(&m_generic, "input", "Connect here the lists to use in the loop"))
	{
		addInput(&m_control);
		addInput(&m_condition);
		addInput(&m_nbIterations);
		addInput(&m_generic);
		addOutput(&m_iteration);

		m_condition.setWidget("checkbox");

		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "init",
											 "Value before the first iteration"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 true, false,
											 "input",
											 "Value to use for the next iteration"));
		defList.push_back(GenericDataDefinition(typeOfList,
											 false, true,
											 "output",
											 "Corresponding value at the previous iteration"));

		setupGenericObject(&m_generic, defList);

		m_laterUpdate = true;
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		VecData* dataInit = dynamic_cast<VecData*>(list[0]);
		VecData* dataInput = dynamic_cast<VecData*>(list[1]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[2]);
		Q_ASSERT(dataInput && dataInit && dataOutput);

		m_parentDocument->setDataDirty(dataOutput);
		if(m_copyInit)
			dataOutput->getAccessor() = dataInit->getValue();
		else
			dataOutput->getAccessor() = dataInput->getValue();
		m_parentDocument->setDataReady(dataOutput);
	}

	void update()
	{
		PReal newControl = m_control.getValue();
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

			cleanDirty();
		}
	}

	void setDirtyValue(const DataNode* caller)
	{
		if(caller == &m_condition)
			return;

		if(m_isUpdating)
			DataNode::setDirtyValue(caller);
		else if(caller == &m_control || caller == this)
			PandaObject::setDirtyValue(caller);
	}

protected:
	bool m_copyInit;
	PReal prevControl;
	Data<PReal> m_control;
	Data<int> m_condition, m_nbIterations, m_iteration;
	GenericVectorData m_generic;
};

int ListLoopClass = RegisterObject<ListLoop>("List/Loop").setDescription("Update values multiple times");

} // namespace Panda

