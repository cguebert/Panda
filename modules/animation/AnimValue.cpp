#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

namespace panda {

class AnimValue : public GenericObject
{
	GENERIC_OBJECT(AnimValue, allAnimationTypes)
public:
	PANDA_CLASS(AnimValue, GenericObject)

    AnimValue(PandaDocument *doc)
		: GenericObject(doc)
        , progress(initData(&progress, "progress", "Position inside the animation"))
        , generic(initData(&generic, "input", "Connect here the animations to get the values from"))
    {
        addInput(&progress);
        addInput(&generic);

		progress.getAccessor().push_back(0.0);

		GenericDataDefinitionList defList;
		// Create an animation of the same type as the data connected
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfAnimation(0),
											 true, false,
											 "input",
											 "Animation from which to extract the values"));
		// Create a list of the same type as the data connected
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfVector(0),
											 false, true,
											 "output",
											 "Values extracted from the animation"));

		setupGenericObject(&generic, defList);
    }

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		typedef Data< Animation<T> > AnimData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		const QVector<double>& key = progress.getValue();
		const Animation<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		int size = key.size();
		outVal.resize(size);
		for(int i=0; i<size; ++i)
			outVal[i] = inVal.get(key[i]);
	}

protected:
    Data< QVector<double> > progress;
    GenericAnimationData generic;
};

int AnimValueClass = RegisterObject("Animation/Animation value").setClass<AnimValue>().setName("Anim value").setDescription("Extract a value from an animation");

} // namespace Panda
