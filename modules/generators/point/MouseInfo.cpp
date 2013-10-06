#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorPoint_MouseInfo : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoint_MouseInfo, PandaObject)

	GeneratorPoint_MouseInfo(PandaDocument *doc)
		: PandaObject(doc)
		, document(doc)
		, position(initData(&position, "position", "Position of the mouse this timestep"))
		, movement(initData(&movement, "movement", "Movement of the mouse from the last timestep"))
		, clickState(initData(&clickState, 0, "click", "1 if the mouse button is pressed"))
	{
		addOutput(&position);
		addOutput(&movement);
		addOutput(&clickState);

		// Connect only 1 input, otherwise update is called twice and movement can't be computed correctly
		BaseData* data = doc->getData("mousePosition");
		if(data) addInput(data);
	}

	void update()
	{
		this->cleanDirty();
		double time = document->getAnimationTime();
		QPointF oldPos = position.getValue(), newPos = document->getMousePosition();

		position.setValue(newPos);

		if(time)
			movement.setValue(newPos - oldPos);
		else
			movement.setValue(QPointF(0,0));

		clickState.setValue(document->getMouseClick());
	}

protected:
	PandaDocument* document;
	Data<QPointF> position, movement;
	Data<int> clickState;
};

int GeneratorPoint_MouseInfoClass = RegisterObject<GeneratorPoint_MouseInfo>("Generator/Point/Mouse").setDescription("Gives information about the mouse in the render view");

} // namespace Panda

