#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorNumber_AnimationTime : public PandaObject
{
public:
	PANDA_CLASS(GeneratorNumber_AnimationTime, PandaObject)

	GeneratorNumber_AnimationTime(PandaDocument *doc)
		: PandaObject(doc)
		, document(doc)
		, animTime(initData(&animTime, 0.0, "time", "Current time of the animation"))
		, timeStep(initData(&timeStep, 0.0, "timestep", "Duration between 2 consecutive timesteps"))
	{
		addOutput(&animTime);
		addOutput(&timeStep);

		BaseData* docTime = doc->getData("time");
		if(docTime)
			addInput(docTime);
	}

	void update()
	{
		animTime.setValue(document->getAnimationTime());
		timeStep.setValue(document->getTimeStep());
		this->cleanDirty();
	}

protected:
	PandaDocument* document;
	Data<double> animTime, timeStep;
};

int GeneratorNumber_AnimationTimeClass = RegisterObject("Generator/Real/Animation time").setClass<GeneratorNumber_AnimationTime>().setName("Anim time").setDescription("Gives the current time of the animation");

} // namespace Panda

