#include <panda/PandaDocument.h>
#include <panda/UpdateLogger.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>

namespace panda {

using types::ImageWrapper;

class GeneratorImage_ConvertToImage : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_ConvertToImage, PandaObject)

	GeneratorImage_ConvertToImage(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input texture or frame buffer (or image)"))
		, m_output(initData("texture", "Output image"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto acc = m_output.getAccessor();
		acc.clear();

		for (const auto& img : input)
		{
			ImageWrapper wrapper;
			wrapper.setImage(img.getImage());
			acc.push_back(wrapper);
		}
	}

protected:
	Data<std::vector<ImageWrapper>> m_input, m_output;
};

int GeneratorImage_ConvertToImageClass = RegisterObject<GeneratorImage_ConvertToImage>("Generator/Image/Convert to image")
	.setDescription("Convert textures to images that can be used on the CPU.");

} // namespace Panda
