#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorPoint_RenderSize : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoint_RenderSize, PandaObject)

	GeneratorPoint_RenderSize(PandaDocument *doc)
		: PandaObject(doc)
		, document(doc)
		, renderSize(initData(&renderSize, "size", "Dimensions of the render"))
	{
		addOutput(&renderSize);

		// Connect only 1 input, otherwise update is called twice and movement can't be computed correctly
		BaseData* data = doc->getData("render size");
		if(data) addInput(data);
	}

	void update()
	{
		this->cleanDirty();
		QSize tmpSize = document->getRenderSize();
		renderSize.setValue(QPointF(tmpSize.width(), tmpSize.height()));
	}

protected:
	PandaDocument* document;
	Data<QPointF> renderSize;
};

int GeneratorPoint_RenderSizeClass = RegisterObject("Generator/Point/Render size").setClass<GeneratorPoint_RenderSize>().setDescription("Gives the dimensions of the render");

} // namespace Panda

