#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Animation.h>

namespace panda {

using types::DataTypeId;
using types::Animation;

class List2Anim : public GenericObject
{
	GENERIC_OBJECT(List2Anim, allAnimationTypes)
public:
	PANDA_CLASS(List2Anim, GenericObject)

	List2Anim(PandaDocument *doc)
		: GenericObject(doc)
		, keys(initData(&keys, "keys", "List of keys for the animations"))
		, interpolation(initData(&interpolation, 0, "interpolation", "Interpolation method between two values"))
		, extend(initData(&extend, 0, "extend", "What to do when a position is outside the boundaries"))
		, generic(initData(&generic, "input", "Connect here the lists of values"))
	{
		addInput(&keys);
		addInput(&interpolation);
		addInput(&extend);
		addInput(&generic);

		interpolation.setWidget("enum_AnimationInterpolation");
		extend.setWidget("enum_AnimationExtend");

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 true, false,
											 "input",
											 "List used to create the animation"));
		// Create an animation of the same type as the data connected
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfAnimation(0),
											 false, true,
											 "output",
											 "Animation created from the given list"));

		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		typedef Data< Animation<T> > AnimData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		AnimData* dataOutput = dynamic_cast<AnimData*>(list[1]);
		Q_ASSERT(dataInput && dataOutput);

		const QVector<T>& inVal = dataInput->getValue();
		auto outVal = dataOutput->getAccessor();

		outVal->setExtend(extend.getValue());
		outVal->setInterpolation(interpolation.getValue());

		const QVector<double>& keysList = keys.getValue();

		int nb = qMin(keysList.size(), inVal.size());
		outVal->clear();
		for(int i=0; i<nb; ++i)
			outVal->add(keysList[i], inVal[i]);
	}

protected:
	Data< QVector<double> > keys;
	Data<int> interpolation, extend;
	GenericVectorData generic;
};

int List2AnimClass = RegisterObject<List2Anim>("Animation/List to Animation").setName("Lists 2 Anim").setDescription("Create animations from lists of keys and values");

//***************************************************************//

class Anim2List : public GenericObject
{
	GENERIC_OBJECT(Anim2List, allAnimationTypes)
public:
	PANDA_CLASS(Anim2List, GenericObject)

	Anim2List(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the animations to get the values from"))
	{
		addInput(&generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfAnimation(0),
											 true, false,
											 "input",
											 "Animation from which to get the values"));
		// Create an animation of the same type as the data connected
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<double>()),
											 false, true,
											 "keys",
											 "List of keys of the given animation"));
		// Create an animation of the same type as the data connected
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 false, true,
											 "values",
											 "List of values of the given animation"));

		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< Animation<T> > AnimData;
		typedef Data< QVector<double> > KeysVecData;
		typedef Data< QVector<T> > ValuesVecData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		KeysVecData* dataKeys = dynamic_cast<KeysVecData*>(list[1]);
		ValuesVecData* dataValues = dynamic_cast<ValuesVecData*>(list[2]);
		Q_ASSERT(dataInput && dataKeys && dataValues);

		const auto& anim = dataInput->getValue();
		dataKeys->getAccessor() = anim.getKeys();
		dataValues->getAccessor() = anim.getValues();
	}

protected:
	GenericAnimationData generic;
};

int Anim2ListClass = RegisterObject<Anim2List>("Animation/Animation to List").setName("Anim 2 Lists").setDescription("Extract the values from an animation");

} // namespace Panda
