#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorImage_Load : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_Load, PandaObject)

	GeneratorImage_Load(PandaDocument *doc)
		: PandaObject(doc)
		, fileName(initData(&fileName, "fileName", "Path of the image to load"))
		, image(initData(&image, "image", "The image loaded from disk"))
	{
		addInput(&fileName);
		fileName.setWidget("open file");

		addOutput(&image);
	}

	void update()
	{
		auto img = image.getAccessor();
		img->load(fileName.getValue());
		this->cleanDirty();
	}

protected:
	Data<QString> fileName;
	Data<QImage> image;
};

int GeneratorImage_LoadClass = RegisterObject<GeneratorImage_Load>("Generator/Image/Load image").setDescription("Load an image from the disk");

} // namespace Panda
