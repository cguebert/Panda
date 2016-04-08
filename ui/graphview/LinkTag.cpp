#include <QPainter>

#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTag.h>

LinkTag::LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output, int index)
	: m_index(index)
	, m_parentView(view)
	, m_inputData(input)
{
	m_outputDatas.emplace(output, std::make_pair(QRectF(),QRectF()));
}

void LinkTag::addOutput(panda::BaseData* output)
{
	if(!m_outputDatas.count(output))
		m_outputDatas.emplace(output, std::make_pair(QRectF(),QRectF()));
}

void LinkTag::removeOutput(panda::BaseData* output)
{
	m_outputDatas.erase(output);
}

void LinkTag::update()
{
	QRectF dataRect = m_parentView->getDataRect(m_inputData);
	m_inputDataRects.first = QRectF(dataRect.right() + tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);
	m_inputDataRects.second = dataRect;

	qreal ix = dataRect.center().x();

	for (auto it = m_outputDatas.begin(); it != m_outputDatas.end();)
	{
		dataRect = m_parentView->getDataRect(it->first);
		qreal ox = dataRect.center().x();
		if (!needLinkTag(ix, ox, m_parentView))
			it = m_outputDatas.erase(it);
		else
		{
			QRectF tagRect(dataRect.left() - tagW - tagMargin,
				dataRect.center().y() - tagH / 2.0,
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

bool LinkTag::containsPoint(const QPointF& point)
{
	if(m_inputDataRects.first.contains(point))
		return true;

	for(auto rect : m_outputDatas)
		if(rect.second.first.contains(point))
			return true;

	return false;
}

void LinkTag::moveView(const QPointF& delta)
{
	m_inputDataRects.first.translate(delta);
	m_inputDataRects.second.translate(delta);
	for (auto& rect : m_outputDatas)
	{
		rect.second.first.translate(delta);
		rect.second.second.translate(delta);
	}
}

void LinkTag::draw(QPainter* painter)
{
	painter->save();
	if(m_hovering)
	{
		painter->setPen(QPen(m_parentView->palette().highlight(), 3));
		painter->setBrush(Qt::NoBrush);

		// Draw as links
		auto inputCenter = m_inputDataRects.second.center();
		for(const auto& tagRect : m_outputDatas)
		{
			auto outputCenter = tagRect.second.second.center();
			double w = (outputCenter.x() - inputCenter.x()) / 2;
			QPainterPath path;
			path.moveTo(inputCenter);
			path.cubicTo(inputCenter + QPointF(w, 0), outputCenter - QPointF(w, 0), outputCenter);
			painter->drawPath(path);
		}

		// Draw the data rectangles
		painter->setBrush(m_parentView->palette().highlight().color());
		painter->setPen(m_parentView->palette().text().color());
		painter->drawRect(m_inputDataRects.second);
		for(const auto& tagRect : m_outputDatas)
			painter->drawRect(tagRect.second.second);
	}
	
	painter->setBrush(m_parentView->palette().light());
	painter->setPen(QPen(m_parentView->palette().text().color()));

	QFont font;
	font.setPointSize(7);
	painter->setFont(font);

	auto indexText = QString::number(m_index + 1);

	// input
	auto inputRect = m_inputDataRects.first;
	qreal x = inputRect.left();
	qreal cy = inputRect.center().y();
	painter->drawLine(x - tagMargin, cy, x, cy);
	painter->drawRect(inputRect);
	painter->drawText(inputRect, Qt::AlignCenter, indexText);

	// outputs
	for(const auto& tagRectPair : m_outputDatas)
	{
		const auto& tagRect = tagRectPair.second.first;
		x = tagRect.right();
		cy = tagRect.center().y();
		painter->drawLine(x, cy, x + tagMargin, cy);
		painter->drawRect(tagRect);
		painter->drawText(tagRect, Qt::AlignCenter, indexText);
	}

	painter->restore();
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