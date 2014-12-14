#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

using types::Point;

class GeneratorPoint_MouseInfo : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoint_MouseInfo, PandaObject)

	GeneratorPoint_MouseInfo(PandaDocument *doc)
		: PandaObject(doc)
		, position(initData(&position, "position", "Position of the mouse this timestep"))
		, movement(initData(&movement, "movement", "Movement of the mouse from the last timestep"))
		, clickState(initData(&clickState, 0, "click", "1 if the mouse button is pressed"))
	{
		addOutput(&position);
		addOutput(&movement);
		addOutput(&clickState);

		clickState.setWidget("checkbox");
	}

	void beginStep()
	{
		setDirtyValue(this);
	}

	void update()
	{
		PReal time = m_parentDocument->getAnimationTime();
		Point oldPos = position.getValue(), newPos = m_parentDocument->getMousePosition();

		position.setValue(newPos);

		if(time)
			movement.setValue(newPos - oldPos);
		else
			movement.setValue(Point(0,0));

		clickState.setValue(m_parentDocument->getMouseClick());
	}

protected:
	Data<Point> position, movement;
	Data<int> clickState;
};

int GeneratorPoint_MouseInfoClass = RegisterObject<GeneratorPoint_MouseInfo>("Generator/Point/Mouse").setDescription("Gives information about the mouse in the render view");

} // namespace Panda
