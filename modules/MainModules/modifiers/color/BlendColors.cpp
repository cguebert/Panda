#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Color.h>

namespace panda {

using types::Color;

class ModifierColor_Blend : public PandaObject
{
public:
	PANDA_CLASS(ModifierColor_Blend, PandaObject)

	ModifierColor_Blend(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData("input 1", "The first color"))
		, inputB(initData("input 2", "The second color"))
		, output(initData("output", "Resulting color"))
		, blendMode(initData("mode", "Method used to blend the colors"))
	{
		addInput(inputA);
		addInput(inputB);
		addInput(blendMode);

		blendMode.setWidget("enum");
		blendMode.setWidgetData("Multiply;Addition;Substraction");

		addOutput(output);
	}

	void update()
	{
		int mode = blendMode.getValue();
		const auto& inListA = inputA.getValue();
		const auto& inListB = inputB.getValue();
		auto outList = output.getAccessor();

		int nbA = inListA.size(), nbB = inListB.size();
		if(nbA && nbB)
		{
			int nb = std::max(nbA, nbB);
			if(nbA < nb) nbA = 1;
			if(nbB < nb) nbB = 1;

			outList.resize(nb);

			switch(mode)
			{
			case 0:	// Multiply
				for(int i=0; i<nb; ++i)
					outList[i] = inListA[i%nbA] * inListB[i%nbB];
				break;
			case 1:	// Addition
				for(int i=0; i<nb; ++i)
					outList[i] = (inListA[i%nbA] + inListB[i%nbB]).bounded();
				break;
			case 2:	// Substraction
				for(int i=0; i<nb; ++i)
					outList[i] = (inListA[i%nbA] - inListB[i%nbB]).bounded();
				break;
			}
		}

		cleanDirty();
	}

protected:
	Data< QVector<Color> > inputA, inputB, output;
	Data< int > blendMode;
};

int ModifierColor_BlendClass = RegisterObject<ModifierColor_Blend>("Modifier/Color/Blend colors").setDescription("Compute a composite of 2 colors");

} // namespace Panda
