#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class ModifierColor_Blend : public PandaObject
{
public:
	PANDA_CLASS(ModifierColor_Blend, PandaObject)

	ModifierColor_Blend(PandaDocument *doc)
		: PandaObject(doc)
		, inputA(initData(&inputA, "input 1", "The first color"))
		, inputB(initData(&inputB, "input 2", "The second color"))
		, output(initData(&output, "output", "Resulting color"))
		, blendMode(initData(&blendMode, "mode", "Method used to blend the colors"))
	{
		addInput(&inputA);
		addInput(&inputB);
		addInput(&blendMode);

		blendMode.setWidget("enum");
		blendMode.setWidgetData("Multiply;Addition;Substraction");

		addOutput(&output);
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
				{
					const QRgb& inA = inListA[i%nbA].rgba();
					const QRgb& inB = inListB[i%nbB].rgba();
					int r1 = qRed(inA), r2 = qRed(inB);
					int g1 = qGreen(inA), g2 = qGreen(inB);
					int b1 = qBlue(inA), b2 = qBlue(inB);
					int a1 = qAlpha(inA), a2 = qAlpha(inB);
					int r = (r1 * r2) >> 8;
					int g = (g1 * g2) >> 8;
					int b = (b1 * b2) >> 8;
					int a = (a1 * a2) >> 8;
					outList[i] = qRgba(r, g, b, a);
				}
				break;
			case 1:	// Addition
				for(int i=0; i<nb; ++i)
				{
					const QRgb& inA = inListA[i%nbA].rgba();
					const QRgb& inB = inListB[i%nbB].rgba();
					int r1 = qRed(inA), r2 = qRed(inB);
					int g1 = qGreen(inA), g2 = qGreen(inB);
					int b1 = qBlue(inA), b2 = qBlue(inB);
					int a1 = qAlpha(inA), a2 = qAlpha(inB);
					int r = (r1 + r2);
					int g = (g1 + g2);
					int b = (b1 + b2);
					int a = (a1 + a2);
					outList[i] = qRgba(r, g, b, a);
				}
				break;
			case 2:	// Substraction
				for(int i=0; i<nb; ++i)
				{
					const QRgb& inA = inListA[i%nbA].rgba();
					const QRgb& inB = inListB[i%nbB].rgba();
					int r1 = qRed(inA), r2 = qRed(inB);
					int g1 = qGreen(inA), g2 = qGreen(inB);
					int b1 = qBlue(inA), b2 = qBlue(inB);
					int a1 = qAlpha(inA), a2 = qAlpha(inB);
					int r = std::max(r1 - r2, 0);
					int g = std::max(g1 - g2, 0);
					int b = std::max(b1 - b2, 0);
					int a = std::max(a1 - a2, 0);
					outList[i] = qRgba(r, g, b, a);
				}
				break;
			}
		}

		cleanDirty();
	}

protected:
	Data< QVector<QColor> > inputA, inputB, output;
	Data< int > blendMode;
};

int ModifierColor_BlendClass = RegisterObject<ModifierColor_Blend>("Modifier/Color/Blend colors").setDescription("Compute a composite of 2 colors");

} // namespace Panda
