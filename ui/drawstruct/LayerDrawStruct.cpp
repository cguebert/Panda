#include <ui/drawstruct/LayerDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/object/Layer.h>

LayerDrawStruct::LayerDrawStruct(GraphView* view, panda::Layer* object)
	: DockObjectDrawStruct(view, object)
	, m_layer(object)
{
	update();
}

panda::types::Rect LayerDrawStruct::getTextArea()
{
	auto textArea = m_visualArea;
	textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
	int margin = dataRectSize+dataRectMargin+3;
	textArea.adjust(margin, 0, -margin, 0);
	return textArea;
}

std::string LayerDrawStruct::getLabel() const
{
	if (m_layer)
	{
		const auto& name = m_layer->getLayerName();
		if (!name.empty())
			return name + "\n(" + m_layer->getName() + ")";
	}
	
	return ObjectDrawStruct::getLabel();
}

int LayerDrawClass = RegisterDrawObject<panda::Layer, LayerDrawStruct>();
