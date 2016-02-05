#include <panda/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Gradient.h>

namespace panda {

using types::Color;
using types::DataTypeId;

class Gradients_Compose : public GenericObject
{
public:
	PANDA_CLASS(Gradients_Compose, GenericObject)

	Gradients_Compose(PandaDocument *doc)
		: GenericObject(doc)
		, nbGradients(initData(1, "# gradients", "Number of gradients to generate"))
		, gradients(initData("output", "Generated gradient"))
		, generic(initData("input", "Connect here the colors to add to the gradient"))
	{
		addInput(nbGradients);
		addInput(generic);

		addOutput(gradients);

		GenericDataDefinitionList defList;
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<Color>()),
											 true, false,
											 "color",
											 "Color to add to the gradient"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfSingleValue(DataTypeId::getIdOf<float>()),
											 true, false,
											 "position",
											 "Position where to insert the color"));
		setupGenericObject<std::tuple<Color>>(this, generic, defList);
	}

	void update()
	{
		tempList.clear();
		tempList.resize(nbGradients.getValue());
		GenericObject::update();
		gradients.setValue(tempList);

		cleanDirty();
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<Color> > VecColorData;
		typedef Data< float > VecRealData;
		VecColorData* dataColor = dynamic_cast<VecColorData*>(list[0]);
		VecRealData* dataPosition = dynamic_cast<VecRealData*>(list[1]);

		assert(dataColor && dataPosition);

		const std::vector<Color>& inColor = dataColor->getValue();
		const float pos = dataPosition->getValue();
		int nbColors = inColor.size();
		int nbGrads = tempList.size();

		if(!nbColors)
			return;

		if(nbColors < nbGrads)
			nbColors = 1;

		for(int i=0; i<nbGrads; ++i)
			tempList[i].add(pos, inColor[i%nbColors]);
	}

protected:
	Data<int> nbGradients;
	Data< std::vector<types::Gradient> > gradients;
	std::vector<types::Gradient> tempList;
	GenericVectorData generic;
};

int Gradients_ComposeClass = RegisterObject<Gradients_Compose>("Generator/Gradient/Compose gradient")
		.setDescription("Create a gradient by adding colors at specific positions");

} // namespace Panda
