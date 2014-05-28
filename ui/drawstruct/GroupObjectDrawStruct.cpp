#include <ui/drawstruct/GroupObjectDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <QPainter>

GroupObjectDrawStruct::GroupObjectDrawStruct(GraphView* view, panda::Group* object)
	: ObjectDrawStruct(view, object)
	, m_group(object)
{
	update();
}

void GroupObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(m_shapePath);
}

void GroupObjectDrawStruct::drawText(QPainter* painter)
{
	if(m_group && !m_group->m_groupName.getValue().isEmpty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = m_objectArea.adjusted(margin, 0, -margin, 0);
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, m_group->m_groupName.getValue());
	}
	else
		ObjectDrawStruct::drawText(painter);
}

void GroupObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	m_shapePath.translate(delta);
}

bool GroupObjectDrawStruct::contains(const QPointF& point)
{
	return m_shapePath.contains(point);
}

void GroupObjectDrawStruct::update()
{
	const int w = 9, h = 9;
	ObjectDrawStruct::update();

	QPainterPath path;
	path.moveTo(m_objectArea.left()+w, m_objectArea.top());
	path.lineTo(m_objectArea.right()-w, m_objectArea.top());
	path.lineTo(m_objectArea.right(), m_objectArea.top()+h);
	path.lineTo(m_objectArea.right(), m_objectArea.bottom()-h);
	path.lineTo(m_objectArea.right()-w, m_objectArea.bottom());
	path.lineTo(m_objectArea.left()+w, m_objectArea.bottom());
	path.lineTo(m_objectArea.left(), m_objectArea.bottom()-h);
	path.lineTo(m_objectArea.left(), m_objectArea.top()+h);
	path.lineTo(m_objectArea.left()+w, m_objectArea.top());
	path.swap(m_shapePath);
}

int GroupObjectDrawStruct::dataStartY()
{
	return 10;
}

int GroupObjectDrawClass = RegisterDrawObject<panda::Group, GroupObjectDrawStruct>();
