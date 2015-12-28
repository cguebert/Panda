#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

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

		int nb = qMin(names.size(), images.size());
		for(int i=0; i<nb; ++i)
		{
			if(!names[i].isEmpty())
				images[i].getImage().save(names[i]);
		}
	}

protected:
	Data< std::vector<QString> > fileName;
	Data< std::vector<ImageWrapper> > image;
	bool inStep;
};

int ModifierImage_SaveClass = RegisterObject<ModifierImage_Save>("File/Image/Save image").setDescription("Save an image to the disk");

} // namespace Panda
