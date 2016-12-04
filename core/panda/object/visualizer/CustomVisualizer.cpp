#include <panda/object/visualizer/CustomVisualizer.h>
#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/data/DataFactory.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/document/Serialization.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/helper/Exception.h>
#include <panda/helper/system/FileRepository.h>

namespace panda
{

CustomVisualizer::CustomVisualizer(PandaDocument* doc)
	: Visualizer(doc)
	, m_docPath(initData("documentPath", "Path to the visualizer document"))
{
	m_docPath.setDisplayed(false);
}

void CustomVisualizer::setDocumentPath(const std::string& path)
{
	m_docPath.setValue(path);

	const auto realPath = panda::helper::system::DataRepository.findFile(path);
	auto doc = panda::serialization::readFile(realPath, parentDocument()->getGUI());
	std::unique_ptr<panda::VisualizerDocument> visuDoc;
	auto visuDocRaw = dynamic_cast<panda::VisualizerDocument*>(doc.get());
	if (visuDocRaw)
	{
		visuDoc.reset(visuDocRaw);
		doc.release();
	}
	else
		throw helper::Exception("CustomVisualizer needs a VisualizerDocument");

	m_initialized = false;
	m_visualizerDocument = std::move(visuDoc);
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

void CustomVisualizer::load(const XmlElement& elem)
{
	Visualizer::load(elem);

	auto path = m_docPath.getValue();
	if (path.empty())
		throw helper::Exception("No document set for CustomVisualizer");
	setDocumentPath(path);
}

int customVisualizerClass = RegisterObject<CustomVisualizer>("Custom visualizer").setDescription("Create a visualizer using a custom document");

} // namespace panda
