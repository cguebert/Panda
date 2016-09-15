#include <panda/document/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

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

//****************************************************************************//

class ModifierImage_SaveMultipleWithThumbnails : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_SaveMultipleWithThumbnails, PandaObject)

	ModifierImage_SaveMultipleWithThumbnails(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path where the image has to be saved"))
		, m_image(initData("image", "The image to be saved"))
		, m_thumbnail(initData("thumbnail", "The thumbnail to save with the image"))
	{
		addInput(m_image);
		addInput(m_thumbnail);
		addInput(m_fileName);

		m_fileName.setWidget("save file");
		m_fileName.setWidgetData(getThumbnailFilterString());

		setUpdateOnMainThread(true);
	}

	static std::string getThumbnailFilterString()
	{
		static std::string filter;
		if (filter.empty())
		{
			filter = "All Files (*);;";
			filter += getFilterString({ FIF_JPEG, FIF_EXR, FIF_TARGA, FIF_TIFF });
		}

		return filter;
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
		const auto& thumbnails = m_thumbnail.getValue();

		if (images.empty())
			return;

		parentDocument()->getGUI().contextMakeCurrent();

		int nb = std::min(names.size(), images.size());
		int nbT = thumbnails.size();
		if (nbT < nb) nbT = 1;

		for (int i = 0; i < nb; ++i)
		{
			const auto& img = images[i].getImage();
			const auto& thumb = thumbnails[i].getImage();
			const auto& fileName = names[i];

			if (img && !img.size().empty() && !fileName.empty())
			{
				auto cpath = fileName.c_str();
				auto fif = FreeImage_GetFIFFromFilename(cpath);
				if (fif != FIF_UNKNOWN
					&& FreeImage_FIFSupportsWriting(fif)
					&& FreeImage_FIFSupportsExportBPP(fif, 32))
				{
					auto dib = convertFromImage(img);
					if (thumb && !thumb.size().empty())
					{
						auto tdib = convertFromImage(thumb);
						FreeImage_SetThumbnail(dib, tdib);
						FreeImage_Unload(tdib);
					}
					FreeImage_Save(fif, dib, cpath, 0);
					FreeImage_Unload(dib);
				}
			}
		}

		parentDocument()->getGUI().contextDoneCurrent();
	}

protected:
	Data< std::vector<std::string> > m_fileName;
	Data< std::vector<ImageWrapper> > m_image, m_thumbnail;
};

int ModifierImage_SaveMultipleWithThumbnailsClass = 
	RegisterObject<ModifierImage_SaveMultipleWithThumbnails>("File/Image/Save with thumbnails")
	.setDescription("Save multiple images and their thumbnails to the disk");


} // namespace Panda
