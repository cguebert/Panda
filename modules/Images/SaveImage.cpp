#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/UpdateLogger.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <algorithm>
#include <modules/Images/utils.h>

namespace panda {

using types::ImageWrapper;

class ModifierImage_Save : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Save, PandaObject)

	ModifierImage_Save(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path where the image has to be saved"))
		, m_image(initData("image", "The image to be saved"))
	{
		addInput(m_image);
		addInput(m_fileName);
		m_fileName.setWidget("save file");
		m_fileName.setWidgetData(getSaveFilterString());

		setUpdateOnMainThread(true);
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

		parentDocument()->getGUI().contextMakeCurrent();
		saveImage(m_image.getValue().getImage(), m_fileName.getValue());
		parentDocument()->getGUI().contextDoneCurrent();
	}

protected:
	Data< std::string > m_fileName;
	Data< ImageWrapper > m_image;
};

int ModifierImage_SaveClass = RegisterObject<ModifierImage_Save>("File/Image/Save image").setDescription("Save an image to the disk");

//****************************************************************************//

class ModifierImage_SaveMultiple : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_SaveMultiple, PandaObject)

	ModifierImage_SaveMultiple(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path where the image has to be saved"))
		, m_image(initData("image", "The image to be saved"))
	{
		addInput(m_image);
		addInput(m_fileName);
		m_fileName.setWidget("save file");
		m_fileName.setWidgetData(getSaveFilterString());

		setUpdateOnMainThread(true);
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
		const auto& names = m_fileName.getValue();
		const auto& images = m_image.getValue();

		if (images.empty())
			return;

		parentDocument()->getGUI().contextMakeCurrent();

		int nb = std::min(names.size(), images.size());
		for(int i=0; i<nb; ++i)
			saveImage(images[i].getImage(), names[i]);

		parentDocument()->getGUI().contextDoneCurrent();
	}

protected:
	Data< std::vector<std::string> > m_fileName;
	Data< std::vector<ImageWrapper> > m_image;
};

int ModifierImage_SaveMultipleClass = RegisterObject<ModifierImage_SaveMultiple>("File/Image/Save images").setDescription("Save multiple images to the disk");

} // namespace Panda