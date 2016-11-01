#include <panda/object/Visualizer.h>
#include <panda/object/ObjectFactory.h>

namespace panda
{

Visualizer::Visualizer(PandaDocument* doc)
	: PandaObject(doc)
	, visualizerSize(initData(types::Point(200, 200), "visualizerSize", "Initial size of the image viewer"))
	, m_aspectRatio(initData(0, "aspectRatio", "Aspect ratio of the image viewer (if 0, not used)"))
	, m_visualizerImage(initData("visualizerImage", "Image to display in the graph view"))
{
	addInput(visualizerSize);
	addInput(m_aspectRatio);
	addOutput(m_visualizerImage);
}

void Visualizer::setDirtyValue(const DataNode* caller)
{
	if(caller == &visualizerSize
	   || (m_visualizedData && caller == m_visualizedData))
		emitModified();

	PandaObject::setDirtyValue(caller);
}

int VisualizerClass = RegisterObject<Visualizer>("Visualizer").setDescription("Create a visualizer for a data");

} // namespace panda
