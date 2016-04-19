#include <ui/drawstruct/GroupDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/object/Group.h>

GroupDrawStruct::GroupDrawStruct(GraphView* view, panda::Group* object)
	: ObjectDrawStruct(view, object)
	, m_group(object)
{
	update();
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

void GroupDrawStruct::createShape()
{
	const int w = 9, h = 9;

	m_outline.clear();
	m_outline.moveTo(pPoint(m_objectArea.left()+w, m_objectArea.top()));
	m_outline.lineTo(pPoint(m_objectArea.right()-w, m_objectArea.top()));
	m_outline.lineTo(pPoint(m_objectArea.right(), m_objectArea.top()+h));
	m_outline.lineTo(pPoint(m_objectArea.right(), m_objectArea.bottom()-h));
	m_outline.lineTo(pPoint(m_objectArea.right()-w, m_objectArea.bottom()));
	m_outline.lineTo(pPoint(m_objectArea.left()+w, m_objectArea.bottom()));
	m_outline.lineTo(pPoint(m_objectArea.left(), m_objectArea.bottom()-h));
	m_outline.lineTo(pPoint(m_objectArea.left(), m_objectArea.top()+h));
	m_outline.lineTo(pPoint(m_objectArea.left()+w, m_objectArea.top()));

	m_fillShape = m_outline.triangulate();
}

int GroupDrawStruct::dataStartY()
{
	return 10;
}

int GroupDrawClass = RegisterDrawObject<panda::Group, GroupDrawStruct>();
