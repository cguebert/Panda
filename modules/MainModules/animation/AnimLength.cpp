#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>
#include <panda/types/Animation.h>

namespace panda {

using types::DataTypeId;
using types::Animation;

class AnimLength : public GenericObject
{
public:
	PANDA_CLASS(AnimLength, GenericObject)

	AnimLength(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the animations to analyse"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		// Create an animation of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfAnimation(0),
							 GenericDataDefinition::Input,
							 "input",
							 "Animation to analyse");
		// Create a single real value
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<float>()),
							 GenericDataDefinition::Output,
							 "length",
							 "Length of this animation");
		// Create a single int value
		defList.emplace_back(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
							 GenericDataDefinition::Output,
							 "size",
							 "Number of keys in this animation");
		setupGenericObject<allAnimationTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< Animation<T> > AnimData;
		typedef Data< float > RealData;
		typedef Data< int > IntData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		RealData* dataLength = dynamic_cast<RealData*>(list[1]);
		IntData* dataSize = dynamic_cast<IntData*>(list[2]);

		assert(dataInput && dataLength && dataSize);

		const Animation<T>& inVal = dataInput->getValue();
		auto keys = inVal.keys();
		dataSize->setValue(keys.size());
		if(!keys.empty())
			dataLength->setValue(keys.back());
	}

protected:
	GenericAnimationData generic;
};

int AnimLengthClass = RegisterObject<AnimLength>("Animation/Animation length").setDescription("Get the length and size of a list of animations");

} // namespace Panda
