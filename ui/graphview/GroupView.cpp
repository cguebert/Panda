#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/graphview/GroupView.h>
#include <ui/graphview/LinkTag.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/PandaDocument.h>
#include <panda/document/ObjectsList.h>
#include <panda/object/Group.h>

using Point = panda::types::Point;
using Rect = panda::types::Rect;

GroupView::GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: GraphView(doc, objectsList, parent)
	, m_group(group)
{
}

void GroupView::paintGL()
{
	GraphView::paintGL();
}

void GroupView::moveObjects(std::vector<panda::PandaObject*> objects, Point delta)
{
	GraphView::moveObjects(objects, delta);

	if(delta.isNull())
		return;

	for (const auto& obj : objects)
	{
		const auto ods = getObjectDrawStruct(obj);
		if (ods)
			m_group->setPosition(obj, ods->getPosition());
	}
}

bool GroupView::isTemporaryView() const
{ 
	return true; 
}
