#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Rect.h>

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
		QSize tmpSize = m_parentDocument->getRenderSize();
		renderSize.setValue(Point(tmpSize.width(), tmpSize.height()));
	}

protected:
	Data<Point> renderSize;
};

int GeneratorPoint_RenderSizeClass = RegisterObject<GeneratorPoint_RenderSize>("Generator/Point/Size of render view").setDescription("Gives the dimensions of the render");

//*************************************************************************//

class GeneratorRect_RenderArea : public PandaObject
{
public:
	PANDA_CLASS(GeneratorRect_RenderArea, PandaObject)

	GeneratorRect_RenderArea(PandaDocument *doc)
		: PandaObject(doc)
		, renderArea(initData(&renderArea, "area", "Area used by the render"))
	{
		addOutput(&renderArea);

		// Connect only 1 input, otherwise update is called twice and movement can't be computed correctly
		BaseData* data = doc->getData("render size");
		if(data) addInput(data);
	}

	void update()
	{
		cleanDirty();
		QSize tmpSize = m_parentDocument->getRenderSize();
		renderArea.setValue(types::Rect(0, 0, tmpSize.width(), tmpSize.height()));
	}

protected:
	Data<types::Rect> renderArea;
};

int GeneratorRect_RenderAreaClass = RegisterObject<GeneratorRect_RenderArea>("Generator/Rectangle/Area of render view").setDescription("Gives the area used by the render");


} // namespace Panda

