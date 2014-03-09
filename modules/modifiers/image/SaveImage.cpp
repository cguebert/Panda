#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

namespace panda {

using types::ImageWrapper;

class ModifierImage_Save : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Save, PandaObject)

	ModifierImage_Save(PandaDocument *doc)
		: PandaObject(doc)
		, fileName(initData(&fileName, "fileName", "Path where the image has to be saved"))
		, image(initData(&image, "image", "The image to be saved"))
	{
		addInput(&image);
		addInput(&fileName);
		fileName.setWidget("save file");
	}

	void setDirtyValue()
	{
		const auto& names = fileName.getValue();
		const auto& images = image.getValue();

		int nb = qMin(names.size(), images.size());
		for(int i=0; i<nb; ++i)
			images[i].getImage().save(names[i]);
	}

protected:
	Data< QVector<QString> > fileName;
	Data< QVector<ImageWrapper> > image;
};

int ModifierImage_SaveClass = RegisterObject<ModifierImage_Save>("Modifier/Image/Save image").setDescription("Save an image to the disk");

} // namespace Panda
