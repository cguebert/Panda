#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/UpdateLogger.h>
#endif

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
		if(!m_isInStep)
			saveImages();
	}

	void saveImages()
	{
#ifdef PANDA_LOG_EVENTS
			helper::ScopedEvent log(helper::event_update, this);
#endif
		const auto& names = fileName.getValue();
		const auto& images = image.getValue();

		int nb = std::min(names.size(), images.size());
		for(int i=0; i<nb; ++i)
		{
			if (!names[i].empty())
			{
				const auto img = images[i].getImage();
				QImage qtImg(img.data(), img.width(), img.height(), QImage::Format_ARGB32);
				qtImg.rgbSwapped().save(QString::fromStdString(names[i]));
			}
		}
	}

protected:
	Data< std::vector<std::string> > fileName;
	Data< std::vector<ImageWrapper> > image;
	bool inStep;
};

int ModifierImage_SaveClass = RegisterObject<ModifierImage_Save>("File/Image/Save image").setDescription("Save an image to the disk");

} // namespace Panda
