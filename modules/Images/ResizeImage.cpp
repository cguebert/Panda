#include <panda/document/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Point.h>

#include <modules/Images/utils.h>

namespace panda {

using types::ImageWrapper;
using types::Point;

class ModifierImage_Resize : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_Resize, PandaObject)

	ModifierImage_Resize(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("image", "The image to be resized"))
		, m_output(initData("output", "The resized image"))
		, m_size(initData("size", "Destination size"))
		, m_filter(initData(0, "filter", "Resampling filter"))
	{
		addInput(m_input);
		addInput(m_size);
		addInput(m_filter);

		addOutput(m_output);

		m_size.getAccessor().push_back(Point(512, 512));

		m_filter.setWidget("enum");
		m_filter.setWidgetData("Box;Bilinear;BSpline;Bicubic;Catmull-Rom;Lanczos");

		setUpdateOnMainThread(true); // If we need to convert a FBO to an image
	}

	void update()
	{
		const auto& inputList = m_input.getValue();
		const auto& sizeList = m_size.getValue();
		auto& outputList = m_output.getAccessor();

		if (!inputList.empty() && !sizeList.empty())
		{
			int nbI = inputList.size(), nbS = sizeList.size();
			auto filter = static_cast<FREE_IMAGE_FILTER>(m_filter.getValue());
			outputList.resize(nbI);
			if (nbS < nbI) nbS = 1;

			for (int i = 0; i < nbI; ++i)
			{
				const auto& input = inputList[i].getImage();
				const auto& size = sizeList[i % nbS];
				auto& output = outputList[i];
				auto dib = convertFromImage(input);

				int dw = static_cast<int>(size.x), dh = static_cast<int>(size.y);

				auto resized = FreeImage_Rescale(dib, dw, dh, filter);
				FreeImage_Unload(dib);

				if (resized)
				{
					output.setImage(convertToImage(resized));
					FreeImage_Unload(resized);
				}
				else
					output.clear();
			}
		}
		else
			outputList.clear();
	}

protected:
	Data<std::vector<ImageWrapper>> m_input, m_output;
	Data<std::vector<Point>> m_size;
	Data<int> m_filter;
};

int ModifierImage_ResizeClass = RegisterObject<ModifierImage_Resize>("Modifier/Image/Resize image").setDescription("Resize an image using a specified filter");

//****************************************************************************//

class ModifierImage_MakeThumbnail : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_MakeThumbnail, PandaObject)

	ModifierImage_MakeThumbnail(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("image", "The image to be resized"))
		, m_output(initData("output", "The resized image"))
		, m_size(initData(100, "size", "Destination size"))
	{
		addInput(m_input);
		addInput(m_size);

		addOutput(m_output);

		setUpdateOnMainThread(true); // If we need to convert a FBO to an image
	}

	void update()
	{
		const auto& inputList = m_input.getValue();
		auto& outputList = m_output.getAccessor();
		const int size = m_size.getValue();

		if (!inputList.empty() && size > 0)
		{
			int nb = inputList.size();
			outputList.resize(nb);

			for (int i = 0; i < nb; ++i)
			{
				const auto& input = inputList[i].getImage();
				auto& output = outputList[i];

				auto dib = convertFromImage(input);
				auto resized = FreeImage_MakeThumbnail(dib, size, TRUE);
				FreeImage_Unload(dib);

				if (resized)
				{
					output.setImage(convertToImage(resized));
					FreeImage_Unload(resized);
				}
				else
					output.clear();
			}
		}
		else
			outputList.clear();
	}

protected:
	Data<std::vector<ImageWrapper>> m_input, m_output;
	Data<int> m_size;
};

int ModifierImage_MakeThumbnailClass = RegisterObject<ModifierImage_MakeThumbnail>("Modifier/Image/Make thumbnail")
	.setDescription("Resize an image using a bilinear filter, keeping aspect ratio");


} // namespace Panda
