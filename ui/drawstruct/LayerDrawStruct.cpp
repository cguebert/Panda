#include <ui/drawstruct/LayerDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/object/Layer.h>

#include <QPainter>

LayerDrawStruct::LayerDrawStruct(GraphView* view, panda::Layer* object)
	: DockObjectDrawStruct(view, object)
	, m_layer(object)
{
	update();
}

void LayerDrawStruct::drawText(QPainter* painter)
{
	if(m_layer && !m_layer->getLayerName().empty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = m_objectArea;
		textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
		textArea.adjust(margin, 0, -margin, 0);
		QString text = QString::fromStdString(m_layer->getLayerName()) + "\n(" + QString::fromStdString(m_layer->getName()) + ")";
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, text);
	}
	else
		DockObjectDrawStruct::drawText(painter);
}

int LayerDrawClass = RegisterDrawObject<panda::Layer, LayerDrawStruct>();
