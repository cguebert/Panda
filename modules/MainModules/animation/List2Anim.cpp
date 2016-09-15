#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/typesLists.h>
#include <panda/types/Animation.h>

namespace panda {

using types::DataTypeId;
using types::Animation;

class List2Anim : public GenericObject
{
public:
	PANDA_CLASS(List2Anim, GenericObject)

	List2Anim(PandaDocument *doc)
		: GenericObject(doc)
		, keys(initData("keys", "List of keys for the animations"))
		, interpolation(initData(0, "interpolation", "Interpolation method between two values"))
		, extend(initData(0, "extend", "What to do when a position is outside the boundaries"))
		, generic(initData("input", "Connect here the lists of values"))
	{
		addInput(keys);
		addInput(interpolation);
		addInput(extend);
		addInput(generic);

		const auto interpolationNames = helper::EasingFunctions::TypeNames();
		std::string interpolationWidgetData;
		for (const auto& name : interpolationNames)
			interpolationWidgetData += name + ";";
		interpolationWidgetData = interpolationWidgetData.substr(0, interpolationWidgetData.size() - 1);

		interpolation.setWidget("enum");
		interpolation.setWidgetData(interpolationWidgetData);

		extend.setWidget("enum");
		extend.setWidgetData("Pad;Repeat;Reflect");

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Input,
							 "input",
							 "List used to create the animation");
		// Create an animation of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfAnimation(0),
							 GenericDataDefinition::Output,
							 "output",
							 "Animation created from the given list");

		setupGenericObject<allAnimationTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > VecData;
		typedef Data< Animation<T> > AnimData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		AnimData* dataOutput = dynamic_cast<AnimData*>(list[1]);
		assert(dataInput && dataOutput);

		const std::vector<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		outVal->setExtendInt(extend.getValue());
		outVal->setInterpolationInt(interpolation.getValue());

		const std::vector<float>& keysList = keys.getValue();

		int nb = std::min(keysList.size(), inVal.size());
		outVal->clear();
		for(int i=0; i<nb; ++i)
			outVal->add(keysList[i], inVal[i]);
	}

protected:
	Data< std::vector<float> > keys;
	Data<int> interpolation, extend;
	GenericVectorData generic;
};

int List2AnimClass = RegisterObject<List2Anim>("Animation/Lists to Animation").setDescription("Create animations from lists of keys and values");

//****************************************************************************//

class Anim2List : public GenericObject
{
public:
	PANDA_CLASS(Anim2List, GenericObject)

	Anim2List(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData("input", "Connect here the animations to get the values from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfAnimation(0),
							 GenericDataDefinition::Input,
							 "input",
							 "Animation from which to get the values");
		// Create an animation of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<float>()),
							 GenericDataDefinition::Output,
							 "keys",
							 "List of keys of the given animation");
		// Create an animation of the same type as the data connected
		defList.emplace_back(DataTypeId::getFullTypeOfVector(0),
							 GenericDataDefinition::Output,
							 "values",
							 "List of values of the given animation");

		setupGenericObject<allAnimationTypes>(this, generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< Animation<T> > AnimData;
		typedef Data< std::vector<float> > KeysVecData;
		typedef Data< std::vector<T> > ValuesVecData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		KeysVecData* dataKeys = dynamic_cast<KeysVecData*>(list[1]);
		ValuesVecData* dataValues = dynamic_cast<ValuesVecData*>(list[2]);
		assert(dataInput && dataKeys && dataValues);

		const auto& anim = dataInput->getValue();
		dataKeys->getAccessor() = anim.keys();
		dataValues->getAccessor() = anim.values();
	}

protected:
	GenericAnimationData generic;
};

int Anim2ListClass = RegisterObject<Anim2List>("Animation/Animation to Lists").setDescription("Extract the values from an animation");

} // namespace Panda
