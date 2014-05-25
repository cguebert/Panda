#include <ui/drawstruct/GroupObjectDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <QPainter>

GroupObjectDrawStruct::GroupObjectDrawStruct(GraphView* view, panda::Group* object)
	: ObjectDrawStruct(view, object)
	, group(object)
{
	update();
}

void GroupObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(shapePath);
}

void GroupObjectDrawStruct::drawText(QPainter* painter)
{
	if(group && !group->groupName.getValue().isEmpty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = m_objectArea.adjusted(margin, 0, -margin, 0);
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, group->groupName.getValue());
	}
	else
		ObjectDrawStruct::drawText(painter);
}

void GroupObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	shapePath.translate(delta);
}

bool GroupObjectDrawStruct::contains(const QPointF& point)
{
	return shapePath.contains(point);
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
	path.swap(shapePath);
}

int GroupObjectDrawStruct::dataStartY()
{
	return 10;
}

int GroupObjectDrawClass = RegisterDrawObject<panda::Group, GroupObjectDrawStruct>();
