#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/data/DataFactory.h>
#include <panda/XmlDocument.h>

namespace panda
{

VisualizerDocument::VisualizerDocument(gui::BaseGUI& gui)
	: RenderedDocument(gui)
	, m_documentDatas(this)
{
}

void VisualizerDocument::setVisualizerType(int type)
{
	m_visualizerType = type;
		
	if (type == -1)
		return;

	auto visuData = DataFactory::create(type, "visualizerData", "The data to visualize", this);
	visuData->setDynamicallyCreated(false); // This object cannot exists without this data
	m_documentDatas.add(visuData);
	addData(visuData.get());
	addInput(*visuData);
}

void VisualizerDocument::save(XmlElement& elem, const std::vector<PandaObject*> *selected)
{
	auto node = elem.addChild("VisualizerData");
	node.setAttribute("type", DataFactory::typeToName(m_visualizerType));

	m_documentDatas.save(elem, "DocumentData");
	RenderedDocument::save(elem, selected);
}

void VisualizerDocument::load(const XmlElement& elem)
{
/*	auto node = elem.firstChild("VisualizerData");
	if(node)
		setVisualizerType(panda::DataFactory::nameToType(node.attribute("type").toString()));
*/
	m_documentDatas.load(elem, "DocumentData");
	auto visuData = getData("visualizerData");
	visuData->setDynamicallyCreated(false);
	RenderedDocument::load(elem);
}

} // namespace panda
