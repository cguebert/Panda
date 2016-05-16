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
#include <panda/types/DataTraits.h>

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
	const int dataRectSize = 10;
	const int dataMarginW = 100;
	const int dataMarginH = 20;

	DrawList list;

	list.addRect(m_objectsRect, m_drawColors.penColor);

	// Count the number of inputs and outputs
	int nbInputs = 0, nbOutputs = 0;
	for (const auto& groupData : m_group->getGroupDatas())
	{
		if (groupData->isInput())
			++nbInputs;
		if (groupData->isOutput())
			++nbOutputs;
	}

	// Where to draw the inputs and outputs
	const int inputsSize = nbInputs * dataRectSize + (nbInputs - 1) * dataMarginH;
	const int outputsSize = nbOutputs * dataRectSize + (nbOutputs - 1) * dataMarginH;
	const float inputsStartY = m_objectsRect.center().y - inputsSize / 2.0f;
	const float outputsStartY = m_objectsRect.center().y - outputsSize / 2.0f;

	auto pen = m_drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& groupData : m_group->getGroupDatas())
	{
		if (groupData->isInput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(m_objectsRect.left() - dataMarginW - dataRectSize, 
												inputsStartY + inputIndex * (dataRectSize + dataMarginH),
												dataRectSize, dataRectSize);
			++inputIndex;

			unsigned int dataColor = DrawList::convert(groupData->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
			list.addRectFilled(groupDataRect, dataColor);
			list.addRect(groupDataRect, m_drawColors.penColor);

			// Draw the tag
			Rect tagRect = Rect::fromSize(groupDataRect.left() - tagW - tagMargin,
										  groupDataRect.center().y - tagH / 2.0,
										  tagW, tagH);

			float x = tagRect.right();
			float cy = tagRect.center().y;
			auto a = tagRect.topLeft();
			auto b = tagRect.bottomLeft();
			auto c = Point(x, cy);
			list.addLine(Point(x, cy - 0.5f), Point(x + tagMargin, cy - 0.5f), m_drawColors.penColor);
			list.addTriangleFilled(a, b, c, m_drawColors.lightColor);
			list.addTriangle(a, b, c, m_drawColors.penColor);

			// Draw links
			auto d1 = groupDataRect.center();
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

					auto d2 = dataRect.center();
					Point w = { (d2.x - d1.x) / 2, 0 };
					list.addBezierCurve(d1, d1 + w, d2 - w, d2, pen, 1);
				}
			}
		}
		
		if (groupData->isOutput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(m_objectsRect.right() + dataMarginW, 
												outputsStartY + outputIndex * (dataRectSize + dataMarginH),
												dataRectSize, dataRectSize);
			++outputIndex;

			unsigned int dataColor = DrawList::convert(groupData->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
			list.addRectFilled(groupDataRect, dataColor);
			list.addRect(groupDataRect, m_drawColors.penColor);

			// Draw the tag
			Rect tagRect = Rect::fromSize(groupDataRect.right() + tagMargin,
										  groupDataRect.center().y - tagH / 2.0,
										  tagW, tagH);

			float x = tagRect.left();
			float cy = tagRect.center().y;
			auto a = tagRect.topLeft();
			auto b = tagRect.bottomLeft();
			auto c = Point(tagRect.right(), cy);
			list.addLine(Point(x - tagMargin, cy), Point(x, cy), m_drawColors.penColor);
			list.addTriangleFilled(a, b, c, m_drawColors.lightColor);
			list.addTriangle(a, b, c, m_drawColors.penColor);

			// Draw links
			auto d2 = groupDataRect.center();
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

					auto d1 = dataRect.center();
					Point w = { (d2.x - d1.x) / 2, 0 };
					list.addBezierCurve(d1, d1 + w, d2 - w, d2, pen, 1);
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
