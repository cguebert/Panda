#include <panda/object/visualizer/VisualizerDocument.h>

namespace panda
{

VisualizerDocument::VisualizerDocument(gui::BaseGUI& gui)
	: RenderedDocument(gui)
{
}

void VisualizerDocument::setVisualizerType(int type)
{
	m_visualizerType = type;
}

} // namespace panda
