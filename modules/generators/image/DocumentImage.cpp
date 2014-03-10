#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

namespace panda {

using types::ImageWrapper;

class GeneratorImage_DocumentRendered : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_DocumentRendered, PandaObject)

	GeneratorImage_DocumentRendered(PandaDocument *doc)
		: PandaObject(doc)
		, previous(initData(&previous, 0, "previous", "If true we get the image at the begining of the timestep, else at the end."))
		, image(initData(&image, "image", "The image rendered by the document"))
	{
		addInput(&previous);
		previous.setWidget("checkbox");

		addOutput(&image);

		BaseData* data = doc->getData("rendered image");
		if(data) addInput(data);
	}

	void beginStep()
	{
		PandaObject::beginStep();
		if(previous.getValue())
		{
			image.getAccessor()->setFbo(parentDocument->getFBO());
			cleanDirty();
		}
	}

	void update()
	{
		image.getAccessor()->setFbo(parentDocument->getFBO());
		cleanDirty();
	}

protected:
	Data<int> previous;
	Data<ImageWrapper> image;
};

int GeneratorImage_DocumentRenderedClass = RegisterObject<GeneratorImage_DocumentRendered>("Generator/Image/Document image")
		.setDescription("Get the last rendered image of the document");

} // namespace Panda
