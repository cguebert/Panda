#include <panda/object/visualizer/CustomVisualizer.h>
#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/data/DataFactory.h>
#include <panda/graphics/Framebuffer.h>

namespace panda
{

CustomVisualizer::CustomVisualizer(PandaDocument* doc)
	: Visualizer(doc)
{
	m_visualizedData = nullptr;
}

void CustomVisualizer::setDocument(std::unique_ptr<VisualizerDocument> document)
{
	m_visualizerDocument = std::move(document);
	m_inputData = DataFactory::create(m_visualizerDocument->visualizerType(),
									  "input",
									  "The data to visualize on the graph view",
									  this);

	addInput(*m_inputData);
}

void CustomVisualizer::update()
{
	const auto& size = m_visualizerDocument->getFBO().size();
	m_aspectRatio.setValue(size.width() / static_cast<float>(size.height()));
}

unsigned int CustomVisualizer::visualizerTextureId() const
{ 
	return m_visualizerDocument->getFBO().id(); 
}

int customVisualizerClass = RegisterObject<CustomVisualizer>("Custom visualizer").setDescription("Create a visualizer using a custom document");

} // namespace panda
