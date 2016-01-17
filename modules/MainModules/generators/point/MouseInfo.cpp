#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

namespace panda {

using types::Point;

class GeneratorPoint_MouseInfo : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoint_MouseInfo, PandaObject)

	GeneratorPoint_MouseInfo(PandaDocument *doc)
		: PandaObject(doc)
		, m_firstTimestep(true)
		, m_position(initData("position", "Position of the mouse this timestep"))
		, m_movement(initData("movement", "Movement of the mouse from the last timestep"))
		, m_clickState(initData(0, "click", "1 if the mouse button is pressed"))
	{
		addOutput(m_position);
		addOutput(m_movement);
		addOutput(m_clickState);

		m_clickState.setWidget("checkbox");

		BaseData* docTime = doc->getData("mouse position");
		if (docTime)
			addInput(*docTime);
	}

	void reset()
	{
		m_firstTimestep = true;
		m_movement.setValue(Point());
		setDirtyValue(this);
	}

	void beginStep()
	{
		PandaObject::beginStep();
		setDirtyValue(this);
	}

	void update()
	{
		Point oldPos = m_position.getValue(), newPos = m_parentDocument->getMousePosition();

		Point delta;
		if(oldPos != newPos)
		{
			m_position.setValue(newPos);
			if(!m_firstTimestep)
				delta = newPos - oldPos;
		}

		if(delta != m_movement.getValue())
			m_movement.setValue(delta);

		m_clickState.setValue(m_parentDocument->getMouseClick());

		m_firstTimestep = false;
	}

protected:
	bool m_firstTimestep;
	Data<Point> m_position, m_movement;
	Data<int> m_clickState;
};

int GeneratorPoint_MouseInfoClass = RegisterObject<GeneratorPoint_MouseInfo>("Generator/Point/Mouse").setDescription("Gives information about the mouse in the render view");

} // namespace Panda

