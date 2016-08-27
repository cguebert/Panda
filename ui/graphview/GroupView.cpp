#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/graphview/GroupView.h>
#include <ui/graphview/LinkTag.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/ObjectsList.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/object/Group.h>
#include <panda/types/DataTraits.h>

#include <QtWidgets>

using Point = panda::types::Point;
using Rect = panda::types::Rect;

namespace
{
	inline panda::types::Point convert(const QPointF& pt)
	{ return panda::types::Point(static_cast<float>(pt.x()), static_cast<float>(pt.y())); }
}

GroupView::GroupView(panda::Group* group, panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: GraphView(doc, objectsList, parent)
	, m_group(group)
{
	updateObjectsRect();
}

void GroupView::paintGL()
{
	GraphView::paintGL();

	m_viewRenderer->newFrame();

// Testing a way to draw the group datas
	DrawList list;

	const auto clickedData = getClickedData();
	auto pen = m_drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& gdr : m_groupDataRects)
	{
		const auto groupData = gdr.first;
		const auto& groupDataRect = gdr.second;
		if (groupData->isInput())
		{
			// Draw the data
			unsigned int dataColor = 0;
			if (clickedData && clickedData != groupData && !canLinkWith(groupData))
				dataColor = m_drawColors.lightColor;
			else
				dataColor = DrawList::convert(groupData->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
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
			unsigned int dataColor = 0;
			if (clickedData && clickedData != groupData && !canLinkWith(groupData))
				dataColor = m_drawColors.lightColor;
			else
				dataColor = DrawList::convert(groupData->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
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

bool GroupView::isCompatible(const panda::BaseData* data1, const panda::BaseData* data2)
{
	if(data1->getOwner() == data2->getOwner())
		return false;

	bool isGroup1 = (data1->getOwner() == m_group);
	bool isGroup2 = (data2->getOwner() == m_group);
	bool isInput1 = isGroup1 ? data1->isOutput() : data1->isInput();
	bool isInput2 = isGroup2 ? data2->isOutput() : data2->isInput();
	bool isOutput1 = isGroup1 ? data1->isInput() : data1->isOutput();
	bool isOutput2 = isGroup2 ? data2->isInput() : data2->isOutput();
	if(isInput1)
	{
		if(!isOutput2)
			return false;
		return data1->validParent(data2);
	}
	else if(isInput2)
	{
		if(!isOutput1)
			return false;
		return data2->validParent(data1);
	}

	return false;
}

void GroupView::computeCompatibleDatas(panda::BaseData* data)
{
	if (data->getOwner() != m_group)
	{
		GraphView::computeCompatibleDatas(data);
		if (data->isInput())
		{
			for (auto groupData : m_group->getGroupDatas())
			{
				if (groupData->isInput())
					m_possibleLinks.insert(groupData.get());
			}
		}
		else if (data->isOutput())
		{
			for (auto groupData : m_group->getGroupDatas())
			{
				if (groupData->isOutput())
					m_possibleLinks.insert(groupData.get());
			}
		}
		return;
	}

	std::vector<panda::BaseData*> forbiddenList;
	if (data->isOutput())
		forbiddenList = panda::graph::extractDatas(panda::graph::computeConnectedOutputNodes(data, false));
	else if(data->isInput())
		forbiddenList = panda::graph::extractDatas(panda::graph::computeConnectedInputNodes(data, false));
	std::sort(forbiddenList.begin(), forbiddenList.end());

	m_possibleLinks.clear();
	for (const auto& object : m_objectsList.get())
	{
		for (const auto linkData : object->getDatas())
		{
			if (isCompatible(data, linkData) 
				&& !std::binary_search(forbiddenList.begin(), forbiddenList.end(), linkData))
				m_possibleLinks.insert(linkData);
		}
	}
}

void GroupView::updateObjectsRect()
{
	if(m_isLoading)
		return;

	m_objectsRect = Rect();
	for(const auto& ods : m_orderedObjectDrawStructs)
		m_objectsRect |= ods->getVisualArea();

	int nbInputs = 0, nbOutputs = 0;
	for (const auto& groupData : m_group->getGroupDatas())
	{
		if (groupData->isInput())	++nbInputs;
		if (groupData->isOutput())	++nbOutputs;
	}

	m_onlyObjectsRect = m_objectsRect;
	if (nbInputs)
		m_objectsRect.adjust(-(dataMarginW + dataRectSize + tagMargin + tagW), 0, 0, 0);
	if (nbOutputs)
		m_objectsRect.adjust(0, 0, dataMarginW + tagMargin + tagW, 0);

	updateViewRect();
	updateGroupDataRects();
}

void GroupView::updateViewRect()
{
	m_viewRect = Rect::fromSize(m_objectsRect.topLeft() * m_zoomFactor, m_objectsRect.size() * m_zoomFactor);
	if(!m_orderedObjectDrawStructs.empty())
		m_viewRect.adjust(-5, -5, 5, 5);

	emit viewModified();
}

void GroupView::updateGroupDataRects()
{
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
	const float inputsStartY = m_onlyObjectsRect.center().y - inputsSize / 2.0f;
	const float outputsStartY = m_onlyObjectsRect.center().y - outputsSize / 2.0f;

	m_groupDataRects.clear();
	auto pen = m_drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& groupData : m_group->getGroupDatas())
	{
		if (groupData->isInput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(m_onlyObjectsRect.left() - dataMarginW - dataRectSize, 
												inputsStartY + inputIndex * (dataRectSize + dataMarginH),
												dataRectSize, dataRectSize);
			++inputIndex;
			m_groupDataRects.emplace_back(groupData.get(), groupDataRect);
		}
		
		if (groupData->isOutput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(m_onlyObjectsRect.right() + dataMarginW, 
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

bool GroupView::createLink(panda::BaseData* data1, panda::BaseData* data2)
{
	panda::BaseData *target = nullptr, *parent = nullptr;
	bool isGroup1 = (data1->getOwner() == m_group);
	bool isGroup2 = (data2->getOwner() == m_group);
	bool isInput1 = isGroup1 ? data1->isOutput() : data1->isInput();
	bool isInput2 = isGroup2 ? data2->isOutput() : data2->isInput();
	bool isOutput1 = isGroup1 ? data1->isInput() : data1->isOutput();
	bool isOutput2 = isGroup2 ? data2->isInput() : data2->isOutput();

	if (isInput1 && isOutput2)
	{
		changeLink(data1, data2);
		return true;
	}
	else if (isInput2 && isOutput1)
	{
		changeLink(data2, data1);
		return true;
	}
	else
		return false;
}

void GroupView::contextMenuEvent(QContextMenuEvent* event)
{
	m_contextMenuData = nullptr;

	Point pos = convert(event->pos()) / m_zoomFactor + m_viewDelta;
	QMenu menu(this);
	int flags = getContextMenuFlags(pos);

	if (m_hoverTimer->isActive())
		m_hoverTimer->stop();

	panda::gui::BaseGUI::Actions actions;

	if (m_contextMenuData)
	{
		if (m_contextMenuData->isInput())
		{
			if (!m_contextMenuData->getParent())
				actions.emplace_back("Add input group data", [this]() { createInputGroupData(); });
		}
		else if (m_contextMenuData->isOutput())
		{
			bool connectedToGroup = false;
			const auto& outputs = m_contextMenuData->getOutputs();
			if (!outputs.empty())
			{
				for (const auto output : outputs)
				{
					auto data = dynamic_cast<panda::BaseData*>(output);
					if (data && data->getOwner() == m_group)
					{
						connectedToGroup = true;
						break;
					}
				}
			}

			if(!connectedToGroup)
				actions.emplace_back("Add output group data", [this]() { createOutputGroupData(); });
		}
	}
	else
	{
		for (const auto& dataRect : m_groupDataRects)
		{
			if (dataRect.second.contains(pos))
			{
				m_contextMenuData = dataRect.first;
				if (m_contextMenuData->isInput())
					actions.emplace_back("Remove input group data", [this]() { removeInputGroupData(); });
				else if (m_contextMenuData->isOutput())
					actions.emplace_back("Remove output group data", [this]() { removeOutputGroupData(); });
				break;
			}
		}
	}

	const auto gPos = event->globalPos();
	const auto posI = panda::graphics::PointInt(gPos.x(), gPos.y());
	m_pandaDocument->getGUI().contextMenu(posI, flags, actions);
}

void GroupView::createInputGroupData()
{

}

void GroupView::createOutputGroupData()
{

}

void GroupView::removeInputGroupData()
{

}

void GroupView::removeOutputGroupData()
{

}
