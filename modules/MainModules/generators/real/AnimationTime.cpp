#include <panda/object/ObjectFactory.h>

namespace panda {

class GeneratorNumber_AnimationTime : public PandaObject
{
public:
	PANDA_CLASS(GeneratorNumber_AnimationTime, PandaObject)

	GeneratorNumber_AnimationTime(PandaDocument *doc)
		: PandaObject(doc)
		, animTime(initData((float)0.0, "time", "Current time of the animation"))
		, timeStep(initData((float)0.0, "timestep", "Duration between 2 consecutive timesteps"))
	{
		addOutput(animTime);
		addOutput(timeStep);

		BaseData* docTime = doc->getData("time");
		if(docTime)
			addInput(*docTime);
	}

	void reset()
	{
		update();
	}

	void update()
	{
		animTime.setValue(parentDocument()->getAnimationTime());
		timeStep.setValue(parentDocument()->getTimeStep());
	}

protected:
	Data<float> animTime, timeStep;
};

int GeneratorNumber_AnimationTimeClass = RegisterObject<GeneratorNumber_AnimationTime>("Generator/Real/Animation time").setDescription("Gives the current time of the animation");

} // namespace Panda

