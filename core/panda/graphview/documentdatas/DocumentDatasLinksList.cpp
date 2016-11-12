#include <panda/graphview/documentdatas/DocumentDatasLinksList.h>

#include <panda/graphview/documentdatas/DocumentDatasView.h>
#include <panda/document/DocumentDatas.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/ObjectsList.h>
#include <panda/graphview/graphics/DrawList.h>
#include <panda/object/PandaObject.h>

namespace panda
{

using Point = types::Point;
using Rect = types::Rect;

namespace graphview
{

	DocumentDatasLinksList::DocumentDatasLinksList(DocumentDatasView& view, DocumentDatas& documentDatas)
		: LinksList(view)
		, m_documentDatasView(view)
		, m_documentDatas(documentDatas)
	{
	}

	bool DocumentDatasLinksList::getDataRect(const BaseData* data, types::Rect& rect)
	{
		if (data->getOwner() != m_documentDatas.parent())
			return LinksList::getDataRect(data, rect);
			
		for (const auto& dataRect : m_documentDatasView.dataRects())
		{
			if (dataRect.first == data)
			{
				rect = dataRect.second;
				return true;
			}
		}

		return false;			
	}

	DocumentDatasLinksList::DataRect DocumentDatasLinksList::getDataAtPos(const types::Point& pt)
	{
		auto res = LinksList::getDataAtPos(pt);
		if (res.first)
			return res;

		for (const auto& dataRect : m_documentDatasView.dataRects())
		{
			if (dataRect.second.contains(pt))
				return dataRect;
		}

		return{ nullptr, Rect() };
	}

	DocumentDatasLinksList::ConnectedDatas DocumentDatasLinksList::getConnectedDatas(BaseData* srcData)
	{
		if (srcData->getOwner() != m_documentDatas.parent())
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
		
	bool DocumentDatasLinksList::createLink(BaseData* data1, BaseData* data2)
	{
		BaseData *target = nullptr, *parent = nullptr;
		bool isDocumentData1 = (data1->getOwner() == m_documentDatas.parent());
		bool isDocumentData2 = (data2->getOwner() == m_documentDatas.parent());
		bool isInput1 = isDocumentData1 ? data1->isOutput() : data1->isInput();
		bool isInput2 = isDocumentData2 ? data2->isOutput() : data2->isInput();
		bool isOutput1 = isDocumentData1 ? data1->isInput() : data1->isOutput();
		bool isOutput2 = isDocumentData2 ? data2->isInput() : data2->isOutput();

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

	bool DocumentDatasLinksList::isCompatible(const BaseData* data1, const BaseData* data2)
	{
		if(data1->getOwner() == data2->getOwner())
			return false;

		bool isDocumentData1 = (data1->getOwner() == m_documentDatas.parent());
		bool isDocumentData2 = (data2->getOwner() == m_documentDatas.parent());
		bool isInput1 = isDocumentData1 ? data1->isOutput() : data1->isInput();
		bool isInput2 = isDocumentData2 ? data2->isOutput() : data2->isInput();
		bool isOutput1 = isDocumentData1 ? data1->isInput() : data1->isOutput();
		bool isOutput2 = isDocumentData2 ? data2->isInput() : data2->isOutput();
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

	void DocumentDatasLinksList::computeCompatibleDatas(BaseData* data)
	{
		if (data->getOwner() != m_documentDatas.parent())
		{
			LinksList::computeCompatibleDatas(data);
			if (data->isInput())
			{
				for (auto data : m_documentDatas.get())
				{
					if (data->isInput())
						m_possibleLinks.insert(data.get());
				}
			}
			else if (data->isOutput())
			{
				for (auto data : m_documentDatas.get())
				{
					if (data->isOutput())
						m_possibleLinks.insert(data.get());
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

	void DocumentDatasLinksList::updateLinks(const graphics::DrawColors& colors)
	{
		LinksList::updateLinks(colors);

		auto pen = colors.penColor;
		int inputIndex = 0, outputIndex = 0;
		for (const auto& gdr : m_documentDatasView.dataRects())
		{
			const auto data = gdr.first;
			const auto& dataRect = gdr.second;
			if (data->isInput())
			{
				auto d1 = dataRect.center();
				for (const auto& output : data->getOutputs())
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
		
			if (data->isOutput())
			{
				auto d2 = dataRect.center();
				for (const auto& input : data->getInputs())
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

} // namespace graphview

} // namespace panda
