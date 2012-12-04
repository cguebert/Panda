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

		addOutput(&image);
		image.setDisplayed(false);
    }

    void update()
    {
		QImage& img = *image.beginEdit();

		img.load(fileName.getValue());

		image.endEdit();
        this->cleanDirty();
    }

protected:
	Data<QString> fileName;
	Data<QImage> image;
};

int GeneratorImage_LoadClass = RegisterObject("Generator/Image/Load image").setClass<GeneratorImage_Load>().setDescription("Load an image from the disk");

} // namespace Panda
