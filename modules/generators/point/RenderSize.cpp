#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

using types::Point;

class GeneratorPoint_RenderSize : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoint_RenderSize, PandaObject)

	GeneratorPoint_RenderSize(PandaDocument *doc)
		: PandaObject(doc)
		, renderSize(initData(&renderSize, "size", "Dimensions of the render"))
	{
		addOutput(&renderSize);

		// Connect only 1 input, otherwise update is called twice and movement can't be computed correctly
		BaseData* data = doc->getData("render size");
		if(data) addInput(data);
	}

	void update()
	{
		cleanDirty();
		QSize tmpSize = parentDocument->getRenderSize();
		renderSize.setValue(Point(tmpSize.width(), tmpSize.height()));
	}

protected:
	Data<Point> renderSize;
};

int GeneratorPoint_RenderSizeClass = RegisterObject<GeneratorPoint_RenderSize>("Generator/Point/Render size").setDescription("Gives the dimensions of the render");

} // namespace Panda

