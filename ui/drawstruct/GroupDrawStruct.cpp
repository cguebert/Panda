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

void GroupDrawStruct::drawShape(DrawList& list, DrawColors& colors)
{
	list.addConvexPolyFilled(m_shapePath, colors.fillColor, false);
	list.addPolyline(m_shapePath, colors.penColor, false, colors.penWidth, true);
}

std::string GroupDrawStruct::getLabel() const
{
	if (m_group)
	{
		const auto& name = m_group->getGroupName();
		if (!name.empty())
			return name;
	}
	
	return ObjectDrawStruct::getLabel();
}

void GroupDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	m_shapePath.translate(pPoint(delta.x(), delta.y()));
}

bool GroupDrawStruct::contains(const QPointF& point)
{
	return m_shapePath.contains(pPoint(point.x(), point.y()));
}

void GroupDrawStruct::update()
{
	const int w = 9, h = 9;
	ObjectDrawStruct::update();

	m_shapePath.clear();
	m_shapePath.moveTo(pPoint(m_objectArea.left()+w, m_objectArea.top()));
	m_shapePath.lineTo(pPoint(m_objectArea.right()-w, m_objectArea.top()));
	m_shapePath.lineTo(pPoint(m_objectArea.right(), m_objectArea.top()+h));
	m_shapePath.lineTo(pPoint(m_objectArea.right(), m_objectArea.bottom()-h));
	m_shapePath.lineTo(pPoint(m_objectArea.right()-w, m_objectArea.bottom()));
	m_shapePath.lineTo(pPoint(m_objectArea.left()+w, m_objectArea.bottom()));
	m_shapePath.lineTo(pPoint(m_objectArea.left(), m_objectArea.bottom()-h));
	m_shapePath.lineTo(pPoint(m_objectArea.left(), m_objectArea.top()+h));
	m_shapePath.lineTo(pPoint(m_objectArea.left()+w, m_objectArea.top()));
}

int GroupDrawStruct::dataStartY()
{
	return 10;
}

int GroupDrawClass = RegisterDrawObject<panda::Group, GroupDrawStruct>();
