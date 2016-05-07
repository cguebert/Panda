#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>
#include <panda/types/Animation.h>

namespace panda {

using types::DataTypeId;
using types::Animation;

class AnimValue : public GenericObject
{
public:
	PANDA_CLASS(AnimValue, GenericObject)

	AnimValue(PandaDocument *doc)
		: GenericObject(doc)
		, progress(initData("progress", "Position inside the animation"))
		, generic(initData("input", "Connect here the animations to get the values from"))
	{
		addInput(progress);
		addInput(generic);

		progress.getAccessor().push_back(0.0);

		GenericDataDefinitionList defList;
		// Create an animation of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfAnimation(0),
							 GenericDataDefinition::Input,
							 "input",
							 "Animation from which to extract the values");
		// Create a list of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Output,
							 "output",
							 "Values extracted from the animation");

		setupGenericObject<allAnimationTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		typedef Data< Animation<T> > AnimData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);

		assert(dataInput && dataOutput);

		const std::vector<float>& key = progress.getValue();
		const Animation<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		int size = key.size();
		outVal.resize(size);
		for(int i=0; i<size; ++i)
			outVal[i] = inVal.get(key[i]);
	}

protected:
	Data< std::vector<float> > progress;
	GenericAnimationData generic;
};

int AnimValueClass = RegisterObject<AnimValue>("Animation/Animation value").setDescription("Extract a value from an animation");

} // namespace Panda
