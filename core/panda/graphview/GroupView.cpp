#include <panda/graphview/object/ObjectRenderer.h>
#include <panda/graphview/object/DockableRenderer.h>
#include <panda/graphview/GroupView.h>
#include <panda/graphview/LinksList.h>
#include <panda/graphview/LinkTag.h>
#include <panda/graphview/ObjectsSelection.h>
#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/ViewGUI.h>
#include <panda/graphview/ViewInteraction.h>
#include <panda/graphview/Viewport.h>
#include <panda/graphview/ViewRenderer.h>
#include <panda/graphview/graphics/DrawList.h>

#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/ObjectsList.h>
#include <panda/document/PandaDocument.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/object/Group.h>
#include <panda/types/DataTraits.h>

namespace
{
	static const int dataRectSize = 10;
	static const int dataMarginW = 100;
	static const int dataMarginH = 20;
	static const float tagW = 18;
	static const float tagH = 13;
	static const float tagMargin = 10;
}

namespace panda
{

using Point = types::Point;
using Rect = types::Rect;

namespace graphview
{

class GroupViewport : public Viewport
{
public:
	GroupViewport(GroupView& view)
		: Viewport(view)
		, m_groupView(view)
	{
	}

	Rect onlyObjectsRect() const
	{ return m_onlyObjectsRect; }

	void updateObjectsRect() override
	{
		if(m_view.isLoading())
			return;

		m_objectsRect = Rect();
		for(const auto& objRnd : m_view.objectRenderers().getOrdered())
			m_objectsRect |= objRnd->getVisualArea();

		int nbInputs = 0, nbOutputs = 0;
		for (const auto& groupData : m_groupView.group()->groupDatas().get())
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
		m_groupView.updateGroupDataRects();
	}

private:
	GroupView& m_groupView;
	types::Rect m_onlyObjectsRect; // Without the group datas
};

//****************************************************************************//

	class GroupLinksList : public LinksList
	{
	public:
		GroupLinksList(GroupView& view)
			: LinksList(view)
			, m_groupView(view)
		{
		}

		bool getDataRect(const BaseData* data, types::Rect& rect) override
		{
			if (data->getOwner() != m_groupView.group())
				return LinksList::getDataRect(data, rect);
			
			for (const auto& groupDataRect : m_groupView.groupDataRects())
			{
				if (groupDataRect.first == data)
				{
					rect = groupDataRect.second;
					return true;
				}
			}

			return false;			
		}

		DataRect getDataAtPos(const types::Point& pt) override
		{
			auto res = LinksList::getDataAtPos(pt);
			if (res.first)
				return res;

			for (const auto& groupDataRect : m_groupView.groupDataRects())
			{
				if (groupDataRect.second.contains(pt))
					return groupDataRect;
			}

			return{ nullptr, Rect() };
		}

		ConnectedDatas getConnectedDatas(BaseData* srcData) override
		{
			if (srcData->getOwner() != m_groupView.group())
				return LinksList::getConnectedDatas(srcData);

			LinksList::Rects rects;
			LinksList::PointsPairs links;

			Rect sourceRect;
			if (getDataRect(srcData, sourceRect))
				rects.push_back(sourceRect);
			else
				return { rects, links };

			// Get outputs
			if (srcData->isInput())
			{
				for (const auto node : srcData->getOutputs())
				{
					BaseData* data = dynamic_cast<BaseData*>(node);
					if (data)
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
			else if (srcData->isOutput())
			{
				BaseData* data = srcData->getParent();
				if (data)
				{
					Rect rect;
					if (getDataRect(data, rect))
					{
						rects.push_back(rect);
						links.emplace_back(sourceRect.center(), rect.center());
					}
				}
			}

			return { rects, links };
		}
		
		bool createLink(BaseData* data1, BaseData* data2) override
		{
			BaseData *target = nullptr, *parent = nullptr;
			bool isGroup1 = (data1->getOwner() == m_groupView.group());
			bool isGroup2 = (data2->getOwner() == m_groupView.group());
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

		bool isCompatible(const BaseData* data1, const BaseData* data2) override
		{
			if(data1->getOwner() == data2->getOwner())
				return false;

			bool isGroup1 = (data1->getOwner() == m_groupView.group());
			bool isGroup2 = (data2->getOwner() == m_groupView.group());
			bool isInput1 = isGroup1 ? data1->isOutput() : data1->isInput();
			bool isInput2 = isGroup2 ? data2->isOutput() : data2->isInput();
			bool isOutput1 = isGroup1 ? data1->isInput() : data1->isOutput();
			bool isOutput2 = isGroup2 ? data2->isInput() : data2->isOutput();
			if (isInput1)
			{
				if (!isOutput2)
					return false;
				return data1->validParent(data2);
			}
			else if (isInput2)
			{
				if (!isOutput1)
					return false;
				return data2->validParent(data1);
			}

			return false;
		}

		void computeCompatibleDatas(BaseData* data) override
		{
			auto group = m_groupView.group();
			if (data->getOwner() != group)
			{
				LinksList::computeCompatibleDatas(data);
				if (data->isInput())
				{
					for (auto groupData : group->groupDatas().get())
					{
						if (groupData->isInput())
							m_possibleLinks.insert(groupData.get());
					}
				}
				else if (data->isOutput())
				{
					for (auto groupData : group->groupDatas().get())
					{
						if (groupData->isOutput())
							m_possibleLinks.insert(groupData.get());
					}
				}
				return;
			}

			std::vector<BaseData*> forbiddenList;
			if (data->isOutput())
				forbiddenList = graph::extractDatas(graph::computeConnectedOutputNodes(data, false));
			else if (data->isInput())
				forbiddenList = graph::extractDatas(graph::computeConnectedInputNodes(data, false));
			std::sort(forbiddenList.begin(), forbiddenList.end());

			m_possibleLinks.clear();
			for (const auto& object : m_view.objectsList().get())
			{
				for (const auto linkData : object->getDatas())
				{
					if (isCompatible(data, linkData)
						&& !std::binary_search(forbiddenList.begin(), forbiddenList.end(), linkData))
						m_possibleLinks.insert(linkData);
				}
			}
		}

	protected:
		void updateLinks(const graphics::DrawColors& colors) override
		{
			LinksList::updateLinks(colors);

			auto pen = colors.penColor;
			int inputIndex = 0, outputIndex = 0;
			for (const auto& gdr : m_groupView.groupDataRects())
			{
				const auto groupData = gdr.first;
				const auto& groupDataRect = gdr.second;
				if (groupData->isInput())
				{
					auto d1 = groupDataRect.center();
					for (const auto& output : groupData->getOutputs())
					{
						if (BaseData* data = dynamic_cast<BaseData*>(output))
						{
							Rect dataRect;
							if (!getDataRect(data, dataRect))
								continue;

							auto d2 = dataRect.center();
							Point w = { (d2.x - d1.x) / 2, 0 };
							m_linksDrawList->addBezierCurve(d1, d1 + w, d2 - w, d2, pen, 1);
						}
					}
				}
		
				if (groupData->isOutput())
				{
					auto d2 = groupDataRect.center();
					for (const auto& input : groupData->getInputs())
					{
						if (BaseData* data = dynamic_cast<BaseData*>(input))
						{
							Rect dataRect;
							if (!getDataRect(data, dataRect))
								continue;

							auto d1 = dataRect.center();
							Point w = { (d2.x - d1.x) / 2, 0 };
							m_linksDrawList->addBezierCurve(d1, d1 + w, d2 - w, d2, pen, 1);
						}
					}
				}
			}
		}

		GroupView& m_groupView;
	};

//****************************************************************************//

	class GroupViewInteraction : public ViewInteraction
	{
	public:
		GroupViewInteraction(GroupView& view)
			: ViewInteraction(view)
			, m_groupView(view)
		{
		}

		void contextMenuEvent(const ContextMenuEvent& event) override
		{
			m_contextMenuData = nullptr;

			Point pos = m_view.viewport().toView(event.pos());
			int flags = getContextMenuFlags(pos);

			TimedFunctions::cancelRun(m_hoverTimerId);

			ViewGui::Actions actions;

			if (m_contextMenuData)
			{
				if (m_contextMenuData->isInput())
				{
					if (!m_contextMenuData->getParent())
						actions.emplace_back("Add input group data", 
											 "Add an new input data for the group, based on and connected to this data", 
											 [view = &m_groupView]() { view->createInputGroupData(); });
				}
				else if (m_contextMenuData->isOutput())
				{
					bool connectedToGroup = false;
					const auto& outputs = m_contextMenuData->getOutputs();
					if (!outputs.empty())
					{
						for (const auto output : outputs)
						{
							auto data = dynamic_cast<BaseData*>(output);
							if (data && data->getOwner() == m_groupView.group())
							{
								connectedToGroup = true;
								break;
							}
						}
					}

					if (!connectedToGroup)
						actions.emplace_back("Add output group data", 
											 "Add an new output data for the group, based on and connected to this data", 
											 [view = &m_groupView]() { view->createOutputGroupData(); });
				}
			}
			else
			{
				for (const auto& dataRect : m_groupView.groupDataRects())
				{
					if (dataRect.second.contains(pos))
					{
						m_contextMenuData = dataRect.first;
						if (m_contextMenuData->isInput())
							actions.emplace_back("Remove input group data",
												 "Disconnect this data and remove it from the group",
												 [this]() {
							auto macro = m_view.document()->getUndoStack().beginMacro("remove input group data");
							m_groupView.removeGroupData(m_contextMenuData);
						});
						else if (m_contextMenuData->isOutput())
							actions.emplace_back("Remove output group data",
												 "Disconnect this data and remove it from the group",
												 [this]() {
							auto macro = m_view.document()->getUndoStack().beginMacro("remove output group data");
							m_groupView.removeGroupData(m_contextMenuData);
						});
						break;
					}
				}
			}

			m_view.gui().contextMenu(event.pos(), flags, actions);
		}

	protected:
		GroupView& m_groupView;
	};

//****************************************************************************//

GroupView::GroupView(Group* group, PandaDocument* doc, ObjectsList& objectsList)
	: GraphView(doc, objectsList)
	, m_group(group)
{
}

std::unique_ptr<GroupView> GroupView::createGroupView(Group* group, PandaDocument* doc, ObjectsList& objectsList)
{
	auto groupView = std::unique_ptr<GroupView>(new GroupView(group, doc, objectsList));
	
	auto& viewRef = *groupView;
	groupView->m_linksList = std::make_unique<GroupLinksList>(viewRef);
	groupView->m_interaction = std::make_unique<GroupViewInteraction>(viewRef);
	groupView->m_viewport = std::make_unique<GroupViewport>(viewRef);

	groupView->initComponents();

	auto& docSignals = doc->getSignals();
	groupView->m_observer.get(docSignals.modifiedObject).connect<GroupView, &GroupView::modifiedObject>(groupView.get());

	return groupView;
}

void GroupView::initializeRenderer(ViewRenderer& viewRenderer)
{
	GraphView::initializeRenderer(viewRenderer);

	m_groupDrawList = std::make_shared<graphics::DrawList>(viewRenderer);
}

void GroupView::drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors)
{
	GraphView::drawGraphView(viewRenderer, drawColors);

// Testing a way to draw the group datas
	auto& drawList = *m_groupDrawList;
	drawList.clear();

	const auto clickedData = interaction().clickedData();
	auto pen = drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& gdr : m_groupDataRects)
	{
		const auto groupData = gdr.first;
		const auto& groupDataRect = gdr.second;
		if (groupData->isInput())
		{
			// Draw the data
			unsigned int dataColor = 0;
			if (clickedData && clickedData != groupData && !linksList().canLinkWith(groupData))
				dataColor = drawColors.lightColor;
			else
				dataColor = graphics::DrawList::convert(groupData->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
			drawList.addRectFilled(groupDataRect, dataColor);
			drawList.addRect(groupDataRect, drawColors.penColor);

			// Draw the tag
			Rect tagRect = Rect::fromSize(groupDataRect.left() - tagW - tagMargin,
										  groupDataRect.center().y - tagH / 2.f,
										  tagW, tagH);

			float x = tagRect.right();
			float cy = tagRect.center().y;
			auto a = tagRect.topLeft();
			auto b = tagRect.bottomLeft();
			auto c = Point(x, cy);
			drawList.addLine(Point(x, cy - 0.5f), Point(x + tagMargin, cy - 0.5f), drawColors.penColor);
			drawList.addTriangleFilled(a, b, c, drawColors.lightColor);
			drawList.addTriangle(a, b, c, drawColors.penColor);
		}
		
		if (groupData->isOutput())
		{
			// Draw the data
			unsigned int dataColor = 0;
			if (clickedData && clickedData != groupData && !linksList().canLinkWith(groupData))
				dataColor = drawColors.lightColor;
			else
				dataColor = graphics::DrawList::convert(groupData->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
			drawList.addRectFilled(groupDataRect, dataColor);
			drawList.addRect(groupDataRect, drawColors.penColor);

			// Draw the tag
			Rect tagRect = Rect::fromSize(groupDataRect.right() + tagMargin,
										  groupDataRect.center().y - tagH / 2.f,
										  tagW, tagH);

			float x = tagRect.left();
			float cy = tagRect.center().y;
			auto a = tagRect.topLeft();
			auto b = tagRect.bottomLeft();
			auto c = Point(tagRect.right(), cy);
			drawList.addLine(Point(x - tagMargin, cy), Point(x, cy), drawColors.penColor);
			drawList.addTriangleFilled(a, b, c, drawColors.lightColor);
			drawList.addTriangle(a, b, c, drawColors.penColor);
		}
	}

	m_viewRenderer->addDrawList(m_groupDrawList);
}

void GroupView::updateGroupDataRects()
{
	// Count the number of inputs and outputs
	int nbInputs = 0, nbOutputs = 0;
	for (const auto& groupData : m_group->groupDatas().get())
	{
		if (groupData->isInput())
			++nbInputs;
		if (groupData->isOutput())
			++nbOutputs;
	}

	const auto onlyObjectsRect = dynamic_cast<GroupViewport*>(&viewport())->onlyObjectsRect();

	// Where to draw the inputs and outputs
	const int inputsSize = nbInputs * dataRectSize + (nbInputs - 1) * dataMarginH;
	const int outputsSize = nbOutputs * dataRectSize + (nbOutputs - 1) * dataMarginH;
	const float inputsStartY = onlyObjectsRect.center().y - inputsSize / 2.0f;
	const float outputsStartY = onlyObjectsRect.center().y - outputsSize / 2.0f;

	m_groupDataRects.clear();
	int inputIndex = 0, outputIndex = 0;
	for (const auto& groupData : m_group->groupDatas().get())
	{
		if (groupData->isInput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(onlyObjectsRect.left() - dataMarginW - dataRectSize, 
												inputsStartY + inputIndex * (dataRectSize + dataMarginH),
												static_cast<float>(dataRectSize), static_cast<float>(dataRectSize));
			++inputIndex;
			m_groupDataRects.emplace_back(groupData.get(), groupDataRect);
		}
		
		if (groupData->isOutput())
		{
			// Draw the data
			Rect groupDataRect = Rect::fromSize(onlyObjectsRect.right() + dataMarginW, 
												outputsStartY + outputIndex * (dataRectSize + dataMarginH),
												static_cast<float>(dataRectSize), static_cast<float>(dataRectSize));
			++outputIndex;
			m_groupDataRects.emplace_back(groupData.get(), groupDataRect);
		}
	}
}

void GroupView::createInputGroupData()
{
	auto& undoStack = m_pandaDocument->getUndoStack();
	auto macro = undoStack.beginMacro("create input group data");

	auto data = interaction().contextMenuData();
	auto newData = m_group->duplicateData(data);
	undoStack.push(std::make_shared<AddDataToGroupCommand>(m_group, newData, true, false));
	auto createdData = newData.get();
	createdData->copyValueFrom(data);
	undoStack.push(std::make_shared<LinkDatasCommand>(data, createdData));
}

void GroupView::createOutputGroupData()
{
	auto& undoStack = m_pandaDocument->getUndoStack();
	auto macro = undoStack.beginMacro("create input group data");

	auto data = interaction().contextMenuData();
	auto newData = m_group->duplicateData(data);
	undoStack.push(std::make_shared<AddDataToGroupCommand>(m_group, newData, false, true));
	undoStack.push(std::make_shared<LinkDatasCommand>(newData.get(), data));
}

void GroupView::removeGroupData(BaseData* data)
{
	auto& undoStack = m_pandaDocument->getUndoStack();

	auto outputs = data->getOutputs();
	for (auto node : outputs)
	{
		auto outData = dynamic_cast<BaseData*>(node);
		if (outData)
			undoStack.push(std::make_shared<LinkDatasCommand>(outData, nullptr));
	}

	undoStack.push(std::make_shared<RemoveDataFromGroupCommand>(m_group, data));
	undoStack.push(std::make_shared<LinkDatasCommand>(data, nullptr));
}

void GroupView::modifiedObject(PandaObject* object)
{
	if(object == m_group)
		updateGroupDataRects();
}

} // namespace graphview

} // namespace panda
