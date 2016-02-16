#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/UpdateLogger.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Image.h>

#include <algorithm>
#include <FreeImage.h>

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
