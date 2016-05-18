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

	DrawList list;

	auto pen = m_drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& gdr : m_groupDataRects)
	{
		const auto groupData = gdr.first;
		const auto& groupDataRect = gdr.second;
		if (groupData->isInput())
		{
			// Draw the data
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
		}
		
		if (groupData->isOutput())
		{
			// Draw the data
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

std::pair<panda::BaseData*, Rect> GroupView::getDataAtPos(const panda::types::Point& pt)
{
	auto res = GraphView::getDataAtPos(pt);
	if (res.first)
		return res;

	for (const auto& groupDataRect : m_groupDataRects)
	{
		if (groupDataRect.second.contains(pt))
			return groupDataRect;
	}

	return{ nullptr, Rect() };
}

bool GroupView::getDataRect(const panda::BaseData* data, panda::types::Rect& rect)
{
	if (data->getOwner() == m_group)
	{
		for (const auto& groupDataRect : m_groupDataRects)
		{
			if (groupDataRect.first == data)
			{
				rect = groupDataRect.second;
				return true;
			}
		}

		return false;
	}
	else
		return GraphView::getDataRect(data, rect);
}

std::pair<GraphView::Rects, GraphView::PointsPairs> GroupView::getConnectedDatas(panda::BaseData* srcData)
{
	if (srcData->getOwner() != m_group)
		return GraphView::getConnectedDatas(srcData);

	GraphView::Rects rects;
	GraphView::PointsPairs links;

	Rect sourceRect;
	if(getDataRect(srcData, sourceRect))
		rects.push_back(sourceRect);
	else
		return{ rects, links };

	// Get outputs
	if(srcData->isInput())
	{
		for(const auto node : srcData->getOutputs())
		{
			panda::BaseData* data = dynamic_cast<panda::BaseData*>(node);
			if(data)
			{
				Rect rect;
				if (getDataRect(data, rect))
				{
					rects.push_back(rect);
					links.emplace_back(rect.center(), sourceRect.center());
				}
			}
		}
	}
	// Or the one input
	else if(srcData->isOutput())
	{
		panda::BaseData* data = srcData->getParent();
		if(data)
		{
			Rect rect;
			if(getDataRect(data, rect))
			{
				rects.push_back(rect);
				links.emplace_back(sourceRect.center(), rect.center());
			}
		}
	}

	return{ rects, links };
}

void GroupView::updateViewRect()
{
	GraphView::updateViewRect();
	updateGroupDataRects();
}

void GroupView::updateGroupDataRects()
{
	const int dataRectSize = 10;
	const int dataMarginW = 100;
	const int dataMarginH = 20;

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

	m_groupDataRects.clear();
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
			m_groupDataRects.emplace_back(groupData.get(), groupDataRect);
		}
		
		if (groupData->isOutput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(m_objectsRect.right() + dataMarginW, 
												outputsStartY + outputIndex * (dataRectSize + dataMarginH),
												dataRectSize, dataRectSize);
			++outputIndex;
			m_groupDataRects.emplace_back(groupData.get(), groupDataRect);
		}
	}
}

void GroupView::updateLinks()
{
	GraphView::updateLinks();

	auto pen = m_drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& gdr : m_groupDataRects)
	{
		const auto groupData = gdr.first;
		const auto& groupDataRect = gdr.second;
		if (groupData->isInput())
		{
			auto d1 = groupDataRect.center();
			for (const auto& output : groupData->getOutputs())
			{
				if (panda::BaseData* data = dynamic_cast<panda::BaseData*>(output))
				{
					Rect dataRect;
					if (!getDataRect(data, dataRect))
						continue;

					auto d2 = dataRect.center();
					Point w = { (d2.x - d1.x) / 2, 0 };
					m_linksDrawList.addBezierCurve(d1, d1 + w, d2 - w, d2, pen, 1);
				}
			}
		}
		
		if (groupData->isOutput())
		{
			auto d2 = groupDataRect.center();
			for (const auto& input : groupData->getInputs())
			{
				if (panda::BaseData* data = dynamic_cast<panda::BaseData*>(input))
				{
					Rect dataRect;
					if (!getDataRect(data, dataRect))
						continue;

					auto d1 = dataRect.center();
					Point w = { (d2.x - d1.x) / 2, 0 };
					m_linksDrawList.addBezierCurve(d1, d1 + w, d2 - w, d2, pen, 1);
				}
			}
		}
	}
}
