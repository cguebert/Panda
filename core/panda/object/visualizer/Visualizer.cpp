#include <panda/object/visualizer/Visualizer.h>

namespace panda
{

Visualizer::Visualizer(PandaDocument* doc)
	: PandaObject(doc)
	, visualizerSize(initData(types::Point(200, 200), "visualizerSize", "Initial size of the image viewer"))
	, m_aspectRatio(initData(0, "aspectRatio", "Aspect ratio of the image viewer (if 0, not used)"))
{
	addInput(visualizerSize);
	addInput(m_aspectRatio);
}

void Visualizer::setDirtyValue(const DataNode* caller)
{
	if(caller == &visualizerSize
	   || (m_visualizedData && caller == m_visualizedData))
		dirtyVisualization.run();

	PandaObject::setDirtyValue(caller);
}

} // namespace panda
