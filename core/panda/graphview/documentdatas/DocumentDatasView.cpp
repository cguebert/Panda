#include <panda/graphview/documentdatas/DocumentDatasView.h>
#include <panda/graphview/documentdatas/DocumentDatasLinksList.h>
#include <panda/graphview/documentdatas/DocumentDatasViewInteraction.h>
#include <panda/graphview/documentdatas/DocumentDatasViewport.h>

#include <panda/graphview/ViewInteraction.h>
#include <panda/graphview/ViewRenderer.h>
#include <panda/graphview/graphics/DrawList.h>

#include <panda/document/DocumentDatas.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/PandaDocument.h>

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

DocumentDatasView::DocumentDatasView(DocumentDatas& documentDatas, PandaDocument* doc, ObjectsList& objectsList)
	: GraphView(doc, objectsList)
	, m_documentDatas(documentDatas)
{
}

std::unique_ptr<DocumentDatasView> DocumentDatasView::createDocumentDatasView(DocumentDatas& documentDatas, PandaDocument* doc, ObjectsList& objectsList)
{
	auto view = std::unique_ptr<DocumentDatasView>(new DocumentDatasView(documentDatas, doc, objectsList));
	
	auto& viewRef = *view;
	view->m_linksList = std::make_unique<DocumentDatasLinksList>(viewRef, documentDatas);
	view->m_interaction = std::make_unique<DocumentDatasViewInteraction>(viewRef, documentDatas);
	view->m_viewport = std::make_unique<DocumentDatasViewport>(viewRef, documentDatas);

	view->initComponents();

	auto& docSignals = doc->getSignals();
	view->m_observer.get(docSignals.modifiedObject).connect<DocumentDatasView, &DocumentDatasView::modifiedObject>(view.get());

	view->m_observer.get(view->m_viewport->modified).connect<DocumentDatasView, &DocumentDatasView::updateDataRects>(view.get());

	return view;
}

void DocumentDatasView::initializeRenderer(ViewRenderer& viewRenderer)
{
	GraphView::initializeRenderer(viewRenderer);

	m_documentDatasDrawList = std::make_shared<graphics::DrawList>(viewRenderer);
}

void DocumentDatasView::drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors)
{
	GraphView::drawGraphView(viewRenderer, drawColors);

	// Drawing the document datas
	auto& drawList = *m_documentDatasDrawList;
	drawList.clear();

	const auto clickedData = interaction().clickedData();
	auto pen = drawColors.penColor;
	int inputIndex = 0, outputIndex = 0;
	for (const auto& dr : m_dataRects)
	{
		const auto data = dr.first;
		const auto& dataRect = dr.second;
		if (data->isInput())
		{
			// Draw the data
			unsigned int dataColor = 0;
			if (clickedData && clickedData != data && !linksList().canLinkWith(data))
				dataColor = drawColors.lightColor;
			else
				dataColor = graphics::DrawList::convert(data->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
			drawList.addRectFilled(dataRect, dataColor);
			drawList.addRect(dataRect, drawColors.penColor);

			// Draw the tag
			Rect tagRect = Rect::fromSize(dataRect.left() - tagW - tagMargin,
										  dataRect.center().y - tagH / 2.f,
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
		
		if (data->isOutput())
		{
			// Draw the data
			unsigned int dataColor = 0;
			if (clickedData && clickedData != data && !linksList().canLinkWith(data))
				dataColor = drawColors.lightColor;
			else
				dataColor = graphics::DrawList::convert(data->getDataTrait()->typeColor()) | 0xFF000000; // Setting alpha to opaque
			drawList.addRectFilled(dataRect, dataColor);
			drawList.addRect(dataRect, drawColors.penColor);

			// Draw the tag
			Rect tagRect = Rect::fromSize(dataRect.right() + tagMargin,
										  dataRect.center().y - tagH / 2.f,
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

	m_viewRenderer->addDrawList(m_documentDatasDrawList);
}

void DocumentDatasView::updateDataRects()
{
	// Count the number of inputs and outputs
	int nbInputs = 0, nbOutputs = 0;
	for (const auto& data : m_documentDatas.get())
	{
		if (data->isInput())
			++nbInputs;
		if (data->isOutput())
			++nbOutputs;
	}

	const auto onlyObjectsRect = dynamic_cast<DocumentDatasViewport*>(&viewport())->onlyObjectsRect();

	// Where to draw the inputs and outputs
	const int inputsSize = nbInputs * dataRectSize + (nbInputs - 1) * dataMarginH;
	const int outputsSize = nbOutputs * dataRectSize + (nbOutputs - 1) * dataMarginH;
	const float inputsStartY = onlyObjectsRect.center().y - inputsSize / 2.0f;
	const float outputsStartY = onlyObjectsRect.center().y - outputsSize / 2.0f;

	m_dataRects.clear();
	int inputIndex = 0, outputIndex = 0;
	for (const auto& data : m_documentDatas.get())
	{
		if (data->isInput())
		{
			// Draw the data
			Rect dataRect = Rect::fromSize(onlyObjectsRect.left() - dataMarginW - dataRectSize, 
												inputsStartY + inputIndex * (dataRectSize + dataMarginH),
												static_cast<float>(dataRectSize), static_cast<float>(dataRectSize));
			++inputIndex;
			m_dataRects.emplace_back(data.get(), dataRect);
		}
		
		if (data->isOutput())
		{
			// Draw the data
			Rect dataRect = Rect::fromSize(onlyObjectsRect.right() + dataMarginW, 
												outputsStartY + outputIndex * (dataRectSize + dataMarginH),
												static_cast<float>(dataRectSize), static_cast<float>(dataRectSize));
			++outputIndex;
			m_dataRects.emplace_back(data.get(), dataRect);
		}
	}
}

void DocumentDatasView::modifiedObject(PandaObject* object)
{
	if(object == m_documentDatas.parent())
		updateDataRects();
}

} // namespace graphview

} // namespace panda
