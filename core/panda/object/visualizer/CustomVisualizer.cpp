#include <panda/object/visualizer/CustomVisualizer.h>
#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/data/DataFactory.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/graphics/Framebuffer.h>

namespace panda
{

CustomVisualizer::CustomVisualizer(PandaDocument* doc)
	: Visualizer(doc)
{
}

void CustomVisualizer::setDocument(std::unique_ptr<VisualizerDocument> document)
{
	m_initialized = false;
	m_visualizerDocument = std::move(document);
	m_inputData = DataFactory::create(m_visualizerDocument->visualizerType(),
									  "input",
									  "The data to visualize on the graph view",
									  this);

	m_visualizedData = m_inputData.get();
	addData(m_inputData.get());
	addInput(*m_inputData);

	m_docInputData = m_visualizerDocument->getData("visualizerData");
}

void CustomVisualizer::update()
{
	if (!m_visualizerDocument)
		return;

	const auto docSize = m_visualizerDocument->getRenderSize();
	const auto visuSize = visualizerSize.getValue();
	const auto visuISize = graphics::Size(static_cast<int>(visuSize.x), static_cast<int>(visuSize.y));
	if (visuISize != docSize)
		m_visualizerDocument->setRenderSize(visuISize);

	if (!m_initialized)
	{
		m_initialized = true;
		m_visualizerDocument->getRenderer().initializeGL();
		m_visualizerDocument->getRenderer().setRenderingMainView(true); // Never release the rendering context after updating
	}
	
	m_docInputData->copyValueFrom(m_visualizedData);

	m_visualizerDocument->updateIfDirty();
}

unsigned int CustomVisualizer::visualizerTextureId() const
{ 
	updateIfDirty();
	return m_visualizerDocument ? m_visualizerDocument->getFBO().texture() : 0; 
}

int customVisualizerClass = RegisterObject<CustomVisualizer>("Custom visualizer").setDescription("Create a visualizer using a custom document");

} // namespace panda
