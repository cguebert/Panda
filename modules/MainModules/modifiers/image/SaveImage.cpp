#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/UpdateLogger.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <QImage>

namespace panda {

using types::ImageWrapper;

class ModifierImage_Save : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Save, PandaObject)

	ModifierImage_Save(PandaDocument *doc)
		: PandaObject(doc)
		, fileName(initData("fileName", "Path where the image has to be saved"))
		, image(initData("image", "The image to be saved"))
		, inStep(false)
	{
		addInput(image);
		addInput(fileName);
		fileName.setWidget("save file");
	}

	void endStep()
	{
		PandaObject::endStep();
		if(isDirty())
			saveImages();
	}

	void setDirtyValue(const DataNode* caller)
	{
		PandaObject::setDirtyValue(caller);
		if(!isInStep())
			saveImages();
	}

	void saveImages()
	{
		helper::ScopedEvent log(helper::event_update, this);
		const auto& names = fileName.getValue();

		parentDocument()->getGUI().contextMakeCurrent();
		const auto& images = image.getValue();

		if (images.empty())
			return;

		int nb = std::min(names.size(), images.size());
		for(int i=0; i<nb; ++i)
		{
			if (!names[i].empty())
			{
				const auto img = images[i].getImage();
				QImage qtImg(img.data(), img.width(), img.height(), QImage::Format_ARGB32);
				qtImg.mirrored().rgbSwapped().save(QString::fromStdString(names[i]));
			}
		}

		parentDocument()->getGUI().contextDoneCurrent();
	}

protected:
	Data< std::vector<std::string> > fileName;
	Data< std::vector<ImageWrapper> > image;
	bool inStep;
};

int ModifierImage_SaveClass = RegisterObject<ModifierImage_Save>("File/Image/Save image").setDescription("Save an image to the disk");

} // namespace Panda
