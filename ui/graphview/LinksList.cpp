#include <ui/graphview/LinksList.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTagsList.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/graphics/DrawColors.h>
#include <ui/graphview/object/ObjectRenderer.h>

#include <panda/UndoStack.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/ObjectsList.h>

using panda::types::Point;
using panda::types::Rect;

namespace graphview
{

	LinksList::LinksList(GraphView& view)
		: m_view(view)
	{
	}

	void LinksList::clear()
	{
		m_recomputeLinks = true;
	}

	void LinksList::onBeginDraw(const graphics::DrawColors& colors)
	{
		if (m_recomputeLinks)
			updateLinks(colors);
	}

	bool LinksList::getDataRect(const panda::BaseData* data, panda::types::Rect& rect)
	{
		auto objRnd = m_view.objectRenderers().get(data->getOwner());
		if (!objRnd)
			return false;
		return objRnd->getDataRect(data, rect);
	}

	LinksList::DataRect LinksList::getDataAtPos(const panda::types::Point& pt)
	{
		const auto objRnd = m_view.objectRenderers().getAtPos(pt);
		if (objRnd)
		{
			panda::BaseData* data = objRnd->getDataAtPos(pt);
			Rect dataRect;
			if (objRnd->getDataRect(data, dataRect))
				return { data, dataRect };
		}

		return { nullptr, Rect() };
	}

	LinksList::ConnectedDatas LinksList::getConnectedDatas(panda::BaseData* srcData)
	{
		LinksList::Rects rects;
		LinksList::PointsPairs links;

		Rect sourceRect;
		if (getDataRect(srcData, sourceRect))
			rects.push_back(sourceRect);
		else
			return { rects, links };

		// Get outputs
		if (srcData->isOutput())
		{
			for (const auto node : srcData->getOutputs())
			{
				panda::BaseData* data = dynamic_cast<panda::BaseData*>(node);
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
		else if (srcData->isInput())
		{
			panda::BaseData* data = srcData->getParent();
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

	void LinksList::updateLinks(const graphics::DrawColors& colors)
	{
		m_recomputeLinks = false;
		m_linksDrawList.clear();

		auto col = colors.penColor;

		for (const auto objRnd : m_view.objectRenderers().getOrdered())
		{
			for (const auto& toDataRect : objRnd->getDataRects())
			{
				panda::BaseData* data = toDataRect.first;
				panda::BaseData* parent = data->getParent();
				if (parent && !data->isOutput())
				{
					Rect fromDataRect;
					auto objRnd =  m_view.objectRenderers().get(parent->getOwner());
					if (objRnd && objRnd->getDataRect(parent, fromDataRect)
						&& ! m_view.linkTagsList().hasLinkTag(parent, data)) // We don't draw the link if there is a LinkTag
					{
						auto d1 = fromDataRect.center(), d2 = toDataRect.second.center();
						Point w = { (d2.x - d1.x) / 2, 0 };
						m_linksDrawList.addBezierCurve(d1, d1 + w, d2 - w, d2, col, 1);
					}
				}
			}
		}
	}

	bool LinksList::createLink(panda::BaseData* data1, panda::BaseData* data2)
	{
		panda::BaseData *target = nullptr, *parent = nullptr;
		if (data1->isInput() && data2->isOutput())
		{
			changeLink(data1, data2);
			return true;
		}
		else if (data2->isInput() && data1->isOutput())
		{
			changeLink(data2, data1);
			return true;
		}
		else
			return false;
	}

	bool LinksList::isCompatible(const panda::BaseData* data1, const panda::BaseData* data2)
	{
		if (data1->getOwner() == data2->getOwner())
			return false;

		if (data1->isInput())
		{
			if (!data2->isOutput())
				return false;
			return data1->validParent(data2);
		}
		else if (data2->isInput())
		{
			if (!data1->isOutput())
				return false;
			return data2->validParent(data1);
		}

		return false;
	}

	void LinksList::computeCompatibleDatas(panda::BaseData* data)
	{
		std::vector<panda::BaseData*> forbiddenList;
		if (data->isInput())
			forbiddenList = panda::graph::extractDatas(panda::graph::computeConnectedOutputNodes(data, false));
		else if (data->isOutput())
			forbiddenList = panda::graph::extractDatas(panda::graph::computeConnectedInputNodes(data, false));
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

	void LinksList::changeLink(panda::BaseData* target, panda::BaseData* parent)
	{
		auto macro = m_view.document()->getUndoStack().beginMacro("change link");
		m_view.document()->getUndoStack().push(std::make_shared<panda::LinkDatasCommand>(target, parent));
	}

} // namespace graphview
