#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/UpdateLogger.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Point.h>
#include <panda/graphics/Image.h>

#include <FreeImage.h>

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
		, m_size(initData(Point(512, 512), "size", "Destination size"))
		, m_filter(initData(0, "filter", "Resampling filter"))
	{
		addInput(m_input);
		addInput(m_size);
		addInput(m_filter);

		addOutput(m_output);

		m_filter.setWidget("enum");
		m_filter.setWidgetData("Box;Bilinear;BSpline;Bicubic;Catmull-Rom;Lanczos");

		setUpdateOnMainThread(true);
	}

	void update()
	{
		helper::ScopedEvent log(helper::event_update, this);

		const auto& input = m_input.getValue().getImage();
		if (input)
		{
			int w = input.width(), h = input.height();
			auto dib = FreeImage_Allocate(w, h, 32);
			auto data = FreeImage_GetBits(dib);

			std::memcpy(data, input.data(), w * h * 4);

			auto dstSize = m_size.getValue();
			int dw = static_cast<int>(dstSize.x), dh = static_cast<int>(dstSize.y);

			auto filter = static_cast<FREE_IMAGE_FILTER>(m_filter.getValue());
			auto resized = FreeImage_Rescale(dib, dw, dh, filter);
			FreeImage_Unload(dib);

			if (resized)
			{
				auto width = FreeImage_GetWidth(resized);
				auto height = FreeImage_GetHeight(resized);
				auto data = FreeImage_GetBits(resized);

				auto img = panda::graphics::Image(width, height, data);
				m_output.getAccessor()->setImage(img);

				FreeImage_Unload(resized);
			}
			else
				m_output.getAccessor()->clear();
		}
		else
			m_output.getAccessor()->clear();

		cleanDirty();
	}

protected:
	Data<ImageWrapper> m_input, m_output;
	Data<Point> m_size;
	Data<int> m_filter;
};

int ModifierImage_ResizeClass = RegisterObject<ModifierImage_Resize>("Modifier/Image/Resize image").setDescription("Resize an image using a specified filter");

} // namespace Panda
