#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/data/DataFactory.h>
#include <panda/XmlDocument.h>

namespace panda
{

VisualizerDocument::VisualizerDocument(gui::BaseGUI& gui)
	: RenderedDocument(gui)
{
}

void VisualizerDocument::setVisualizerType(int type)
{
	m_visualizerType = type;
		
	if (type == -1)
		return;
}

void VisualizerDocument::save(XmlElement& elem, const std::vector<PandaObject*> *selected)
{
	auto node = elem.addChild("VisualizerData");
	node.setAttribute("type", DataFactory::typeToName(m_visualizerType));

	RenderedDocument::save(elem, selected);
}

void VisualizerDocument::load(const XmlElement& elem)
{
	auto node = elem.firstChild("VisualizerData");
	if(node)
		setVisualizerType(panda::DataFactory::nameToType(node.attribute("type").toString()));

	RenderedDocument::load(elem);
}

} // namespace panda
