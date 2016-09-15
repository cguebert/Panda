#include <panda/document/RenderedDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

namespace panda {

using types::Point;

class GeneratorPoint_RenderSize : public PandaObject
{
public:
	PANDA_CLASS(GeneratorPoint_RenderSize, PandaObject)

	GeneratorPoint_RenderSize(PandaDocument *doc)
		: PandaObject(doc)
		, renderSize(initData("size", "Dimensions of the render"))
	{
		addOutput(renderSize);

		// Connect only 1 input, otherwise update is called twice and movement can't be computed correctly
		BaseData* data = doc->getData("render size");
		if(data) addInput(*data);
	}

	void update()
	{
		auto tmpSize = parent<RenderedDocument>()->getRenderSize();
		renderSize.setValue(Point(static_cast<float>(tmpSize.width()), static_cast<float>(tmpSize.height())));
	}

protected:
	Data<Point> renderSize;
};

int GeneratorPoint_RenderSizeClass = RegisterObject<GeneratorPoint_RenderSize, RenderedDocument>("Generator/Point/Size of render view").setDescription("Gives the dimensions of the render");

//****************************************************************************//

class GeneratorRect_RenderArea : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRect_RenderArea, PandaObject)

	GeneratorRect_RenderArea(PandaDocument *doc)
		: PandaObject(doc)
		, renderArea(initData("area", "Area used by the render"))
	{
		addOutput(renderArea);

		// Connect only 1 input, otherwise update is called twice and movement can't be computed correctly
		BaseData* data = doc->getData("render size");
		if(data) addInput(*data);
	}

	void update()
	{
		auto tmpSize = parent<RenderedDocument>()->getRenderSize();
		renderArea.setValue(types::Rect(0, 0, static_cast<float>(tmpSize.width()), static_cast<float>(tmpSize.height())));
	}

protected:
	Data<types::Rect> renderArea;
};

int GeneratorRect_RenderAreaClass = RegisterObject<GeneratorRect_RenderArea, RenderedDocument>("Generator/Rectangle/Area of render view").setDescription("Gives the area used by the render");


} // namespace Panda

