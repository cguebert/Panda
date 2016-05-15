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

	m_viewRenderer->newFrame();
// Testing a way to draw the group datas
	const int tagW = 18;
	const int tagH = 13;
	const int tagMargin = 10;

	DrawList list;
	for (const auto& groupData : m_group->getGroupDatas())
	{
		if (groupData->isInput())
		{
			for (const auto& output : groupData->getOutputs())
			{
				if (panda::BaseData* data = dynamic_cast<panda::BaseData*>(output))
				{
					const auto ods = getObjectDrawStruct(data->getOwner());
					if (!ods)
						continue;

					Rect dataRect;
					if (!ods->getDataRect(data, dataRect))
						continue;

					Rect tagRect = Rect::fromSize(dataRect.left() - tagW - tagMargin,
												  dataRect.center().y - tagH / 2.0,
												  tagW, tagH);

					float x = tagRect.right();
					float cy = tagRect.center().y;
					auto a = tagRect.topLeft();
					auto b = tagRect.bottomLeft();
					auto c = Point(x, cy);
					list.addLine(Point(x, cy - 0.5f), Point(x + tagMargin, cy - 0.5f), m_drawColors.penColor);
					list.addTriangleFilled(a, b, c, m_drawColors.lightColor);
					list.addTriangle(a, b, c, m_drawColors.penColor);
				}
			}
		}
		
		if (groupData->isOutput())
		{
			for (const auto& input : groupData->getInputs())
			{
				if (panda::BaseData* data = dynamic_cast<panda::BaseData*>(input))
				{
					const auto ods = getObjectDrawStruct(data->getOwner());
					if (!ods)
						continue;

					Rect dataRect;
					if (!ods->getDataRect(data, dataRect))
						continue;

					Rect tagRect = Rect::fromSize(dataRect.right() + tagMargin,
												  dataRect.center().y - tagH / 2.0,
												  tagW, tagH);

					float x = tagRect.left();
					float cy = tagRect.center().y;
					auto a = tagRect.topLeft();
					auto b = tagRect.bottomLeft();
					auto c = Point(tagRect.right(), cy);
					list.addLine(Point(x - tagMargin, cy), Point(x, cy), m_drawColors.penColor);
					list.addTriangleFilled(a, b, c, m_drawColors.lightColor);
					list.addTriangle(a, b, c, m_drawColors.penColor);
				}
			}
		}
	}

	m_viewRenderer->addDrawList(&list);
	m_viewRenderer->render();
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
