#include <ui/drawstruct/GroupDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/object/Group.h>

#include <QPainter>

GroupDrawStruct::GroupDrawStruct(GraphView* view, panda::Group* object)
	: ObjectDrawStruct(view, object)
	, m_group(object)
{
	update();
}

void GroupDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(m_shapePath);
}

void GroupDrawStruct::drawText(QPainter* painter)
{
	if(m_group && !m_group->getGroupName().empty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = m_objectArea.adjusted(margin, 0, -margin, 0);
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, QString::fromStdString(m_group->getGroupName()));
	}
	else
		ObjectDrawStruct::drawText(painter);
}

void GroupDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	m_shapePath.translate(delta);
}

bool GroupDrawStruct::contains(const QPointF& point)
{
	return m_shapePath.contains(point);
}

void GroupDrawStruct::update()
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

int GroupDrawStruct::dataStartY()
{
	return 10;
}

int GroupDrawClass = RegisterDrawObject<panda::Group, GroupDrawStruct>();
