#include <panda/object/Visualizer.h>
#include <panda/object/ObjectFactory.h>

namespace panda
{

Visualizer::Visualizer(PandaDocument* doc)
	: PandaObject(doc)
	, m_aspectRatio(initData(0, "aspectRatio", "Aspect ratio of the image viewer (if 0, not used)"))
	, m_sizeHint(initData(types::Point(200, 200), "sizeHint", "Initial size of the image viewer"))
	, m_visualizerImage(initData("visualizerImage", "Image to display in the graph view"))
{
	addOutput(m_aspectRatio);
	addOutput(m_sizeHint);
	addOutput(m_visualizerImage);
}

int VisualizerClass = RegisterObject<Visualizer>("Visualizer").setDescription("Create a visualizer for a data");

} // namespace panda
