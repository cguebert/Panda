#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTag.h>
#include <ui/graphview/object/ObjectRenderer.h>

using panda::types::Point;
using panda::types::Rect;

LinkTag::LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output, int index)
	: m_index(index)
	, m_parentView(view)
	, m_inputData(input)
{
	m_outputDatas.emplace(output, std::make_pair(Rect(),Rect()));
}

void LinkTag::addOutput(panda::BaseData* output)
{
	if(!m_outputDatas.count(output))
		m_outputDatas.emplace(output, std::make_pair(Rect(),Rect()));
}

void LinkTag::removeOutput(panda::BaseData* output)
{
	m_outputDatas.erase(output);
}

void LinkTag::update()
{
	Rect dataRect;
	auto ods = m_parentView->getObjectRenderer(m_inputData->getOwner());
	if (!ods)
		return;

	ods->getDataRect(m_inputData, dataRect);
	m_inputDataRects.first = Rect::fromSize(dataRect.right() + tagMargin,
											dataRect.center().y - tagH / 2.0,
											tagW, tagH);
	m_inputDataRects.second = dataRect;

	float ix = dataRect.center().x;

	for (auto it = m_outputDatas.begin(); it != m_outputDatas.end();)
	{
		ods = m_parentView->getObjectRenderer(it->first->getOwner());
		if (!ods)
			continue;
		ods->getDataRect(it->first, dataRect);
		float ox = dataRect.center().x;
		if (!needLinkTag(ix, ox, m_parentView))
			it = m_outputDatas.erase(it);
		else
		{
			Rect tagRect = Rect::fromSize(dataRect.left() - tagW - tagMargin,
										  dataRect.center().y - tagH / 2.0,
										  tagW, tagH);
			it->second.first = tagRect;
			it->second.second = dataRect;
			++it;
		}
	}
}

bool LinkTag::isEmpty()
{
	return m_outputDatas.empty();
}

std::pair<panda::BaseData*, panda::types::Rect> LinkTag::getDataAtPoint(const Point& point)
{
	if (m_inputDataRects.first.contains(point))
		return { m_inputData, m_inputDataRects.first };

	for(auto rect : m_outputDatas)
		if(rect.second.first.contains(point))
			return std::make_pair(rect.first, rect.second.first);

	return { nullptr, Rect() };
}

void LinkTag::moveView(const Point& delta)
{
	m_inputDataRects.first.translate(delta);
	m_inputDataRects.second.translate(delta);
	for (auto& rect : m_outputDatas)
	{
		rect.second.first.translate(delta);
		rect.second.second.translate(delta);
	}
}

void LinkTag::draw(DrawList& list, DrawColors& colors)
{
	if(m_hovering)
	{
		// Draw as links
		auto inputCenter = m_inputDataRects.second.center();
		for(const auto& tagRect : m_outputDatas)
		{
			auto outputCenter = tagRect.second.second.center();
			auto w = Point((outputCenter.x - inputCenter.x) / 2, 0);
			list.addBezierCurve(inputCenter, inputCenter + w, outputCenter - w, outputCenter, colors.highlightColor, 3.0f);
		}

		// Draw the data rectangles
		const auto& rect = m_inputDataRects.second;
		list.addRectFilled(rect, colors.highlightColor);
		list.addRect(rect, colors.penColor);
		for (const auto& tagRect : m_outputDatas)
		{
			const auto& tr = tagRect.second.second;
			list.addRectFilled(tr, colors.highlightColor);
			list.addRect(tr, colors.penColor);
		}
	}

	auto indexText = std::to_string(m_index + 1);
	const float fontScale = 0.85f;

	// input
	auto inputRect = m_inputDataRects.first;
	float x = inputRect.left();
	float cy = inputRect.center().y;
	list.addLine(Point(x - tagMargin, cy), Point(x, cy), colors.penColor);
	list.addRectFilled(inputRect, colors.lightColor);
	list.addRect(inputRect, colors.penColor);
	list.addText(inputRect, indexText, colors.penColor, DrawList::Align_Center, fontScale);

	// outputs
	for(const auto& tagRectPair : m_outputDatas)
	{
		const auto& tagRect = tagRectPair.second.first;
		x = tagRect.right();
		cy = tagRect.center().y;
		list.addLine(Point(x, cy), Point(x + tagMargin, cy), colors.penColor);
		list.addRectFilled(tagRect, colors.lightColor);
		list.addRect(tagRect, colors.penColor);
		list.addText(tagRect, indexText, colors.penColor, DrawList::Align_Center, fontScale);
	}
}

bool LinkTag::needLinkTag(float inputX, float outputX, GraphView* view)
{
	return outputX <= inputX || outputX > inputX + view->width() * 2 / 3;
}

std::vector<panda::BaseData*> LinkTag::getOutputDatas() const
{
	std::vector<panda::BaseData*> res;
	for (const auto dataRect : m_outputDatas)
		res.push_back(dataRect.first);
	return res;
}