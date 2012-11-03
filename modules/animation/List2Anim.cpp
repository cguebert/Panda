#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

namespace panda {

class List2Anim : public GenericObject
{
	GENERIC_OBJECT(List2Anim, allAnimationTypes)
public:
    List2Anim(PandaDocument *doc)
		: GenericObject(doc)
		, interval(initData(&interval, 1.0, "interval", "Interval between 2 animation keys"))
        , generic(initData(&generic, "input", "Connect here the lists to get the values from"))
    {
        addInput(&interval);
        addInput(&generic);

		GenericDataDefinitionList defList;
		// Create a list of the same type as the data connected
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfVector(0),
											 true, false,
											 "input",
											 "List used to create the animation"));
		// Create an animation of the same type as the data connected
		defList.append(GenericDataDefinition(BaseData::getFullTypeOfAnimation(0),
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
		Animation<T>& outVal = *(dataOutput->beginEdit());

		double inter = interval.getValue();
		int size = inVal.size();
		outVal.clear();
		for(int i=0; i<size; ++i)
			outVal.add(i*inter, inVal[i]);

		dataOutput->endEdit();
	}

protected:
	Data<double> interval;
    GenericVectorData generic;
};

int List2AnimClass = RegisterObject("Animation/List to Animation").setClass<List2Anim>().setName("List 2 Anim").setDescription("Create animations from lists");

} // namespace Panda
