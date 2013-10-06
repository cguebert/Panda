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
		, interval(initData(&interval, 1.0, "interval", "Interval between 2 animation keys"))
		, generic(initData(&generic, "input", "Connect here the lists to get the values from"))
	{
		addInput(&interval);
		addInput(&generic);

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

		double inter = interval.getValue();
		int size = inVal.size();
		outVal->clear();
		for(int i=0; i<size; ++i)
			outVal->add(i*inter, inVal[i]);
	}

protected:
	Data<double> interval;
	GenericVectorData generic;
};

int List2AnimClass = RegisterObject<List2Anim>("Animation/List to Animation").setName("List 2 Anim").setDescription("Create animations from lists");

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
		defList.append(GenericDataDefinition(DataTypeId::getFullTypeOfVector(0),
											 false, true,
											 "output",
											 "List created from the given animation"));

		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< Animation<T> > AnimData;
		typedef Data< QVector<T> > VecData;
		AnimData* dataInput = dynamic_cast<AnimData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		Q_ASSERT(dataInput && dataOutput);

		dataOutput->getAccessor() = dataInput->getValue().getValues();
	}

protected:
	GenericAnimationData generic;
};

int Anim2ListClass = RegisterObject<Anim2List>("Animation/Animation to List").setName("Anim 2 List").setDescription("Extract the values from an animation");

} // namespace Panda
