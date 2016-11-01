#include <panda/object/visualizer/ImageVisualizer.h>
#include <panda/object/ObjectFactory.h>

namespace panda
{

ImageVisualizer::ImageVisualizer(PandaDocument* doc)
	: Visualizer(doc)
	, m_input(initData("input", "The data to visualize on the graph view"))
{
	addInput(m_input);

	m_visualizedData = &m_input;
}

void ImageVisualizer::update()
{
	const auto size = m_input.getValue().size();
	m_aspectRatio.setValue(size.width() / static_cast<float>(size.height()));
}

int ImageVisualizerClass = RegisterObject<ImageVisualizer>("Image visualizer").setDescription("Create a visualizer for an image data");

} // namespace panda
