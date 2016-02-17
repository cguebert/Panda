#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/UpdateLogger.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <algorithm>
#include <sstream>
#include <FreeImage.h>

namespace
{

std::string convertExtensionList(const std::string& list)
{
	std::stringstream ss(list);
	std::string ret, item;
	while (std::getline(ss, item, ','))
		ret += (ret.empty() ? "*." : " *.") + item;

	return ret;
}

std::string buildSaveFilterString()
{
	const int nbFIF = FreeImage_GetFIFCount();
	std::string filter;
	// Build a string for 'All image files'
/*	filter += "All image files (";
	bool first = true;
	for (int i = 0; i < nbFIF; ++i)
	{
		auto fif = static_cast<FREE_IMAGE_FORMAT>(i);
		if (FreeImage_FIFSupportsWriting(fif))
		{
			if (!first)
				filter += " ";
			first = false;
			filter += convertExtensionList(FreeImage_GetFIFExtensionList(fif));
		}
	}
	filter += ");;";
	*/
	// Build a string for 'All files'
	filter += "All Files (*)";

	// Build a string for each format
	for (int i = 0; i < nbFIF; ++i)
	{
		auto fif = static_cast<FREE_IMAGE_FORMAT>(i);
		if (FreeImage_FIFSupportsWriting(fif)) 
		{
			filter += ";;";
			filter += FreeImage_GetFIFDescription(fif);
			filter += " (" + convertExtensionList(FreeImage_GetFIFExtensionList(fif)) + ")";
		}
	}

	return filter;
}

std::string getSaveFilterString()
{
	static std::string filter;
	if (filter.empty())
		filter = buildSaveFilterString();
	return filter;
}

}

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
		fileName.setWidgetData(getSaveFilterString());
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
				if (img && !img.size().empty())
				{
					auto cpath = names[i].c_str();
					auto fif = FreeImage_GetFIFFromFilename(cpath);
					if (fif != FIF_UNKNOWN 
						&& FreeImage_FIFSupportsWriting(fif) 
						&& FreeImage_FIFSupportsExportBPP(fif, 32))
					{
						int w = img.width(), h = img.height();
						auto dib = FreeImage_Allocate(w, h, 32);
						auto data = FreeImage_GetBits(dib);

						std::memcpy(data, img.data(), w * h * 4);
						FreeImage_Save(fif, dib, cpath, 0);
						FreeImage_Unload(dib);
					}
				}
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
