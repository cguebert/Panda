#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

namespace panda {

class AnimLength : public GenericObject
{
	GENERIC_OBJECT(AnimLength, allAnimationTypes)
public:
	PANDA_CLASS(AnimLength, GenericObject)

	AnimLength(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the animations to analyse"))
	{
		addInput(&generic);

		GenericDataDefinitionList defList;
		// Create an animation of the same type as the data connected
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfAnimation(0),
											 true, false,
											 "input",
											 "Animation to analyse"));
		// Create a single double value
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<double>()),
											 false, true,
											 "length",
											 "Length of this animation"));
		// Create a single int value
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<int>()),
											 false, true,
											 "size",
											 "Number of keys in this animation"));
		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< Animation<T> > AnimData;
		typedef Data< double > DoubleData;
		typedef Data< int > IntData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		DoubleData* dataLength = dynamic_cast<DoubleData*>(list[1]);
		IntData* dataSize = dynamic_cast<IntData*>(list[2]);

		Q_ASSERT(dataInput && dataLength && dataSize);

		const Animation<T>& inVal = dataInput->getValue();
		QList<double> keys = inVal.getKeys();
		dataSize->setValue(keys.size());
		if(!keys.empty())
			dataLength->setValue(keys.back());
	}

protected:
	GenericAnimationData generic;
};

int AnimLengthClass = RegisterObject("Animation/Animation length").setClass<AnimLength>().setName("Anim length").setDescription("Get the length and size of a list of animations");

} // namespace Panda
