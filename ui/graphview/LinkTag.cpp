#include <QPainter>

#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTag.h>

LinkTag::LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output, int index)
	: m_index(index)
	, m_parentView(view)
	, m_inputData(input)
{
	m_outputDatas.emplace(output, QRectF());
}

void LinkTag::addOutput(panda::BaseData* output)
{
	if(!m_outputDatas.count(output))
		m_outputDatas.emplace(output, QRectF());
}

void LinkTag::removeOutput(panda::BaseData* output)
{
	m_outputDatas.erase(output);
}

void LinkTag::update()
{
	QRectF dataRect = m_parentView->getDataRect(m_inputData);
	m_inputDataRect = QRectF(dataRect.right() + tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);

	qreal ix = dataRect.center().x();

	for (auto it = m_outputDatas.begin(); it != m_outputDatas.end();)
	{
		dataRect = m_parentView->getDataRect(it->first);
		qreal ox = dataRect.center().x();
		if (!needLinkTag(ix, ox))
			it = m_outputDatas.erase(it);
		else
		{
			QRectF tagRect(dataRect.left() - tagW - tagMargin,
				dataRect.center().y() - tagH / 2.0,
				tagW, tagH);
			it->second = tagRect;
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
	if(m_inputDataRect.contains(point))
		return true;

	for(auto rect : m_outputDatas)
		if(rect.second.contains(point))
			return true;

	return false;
}

void LinkTag::moveView(const QPointF& delta)
{
	m_inputDataRect.translate(delta);
	for(auto& rect : m_outputDatas)
		rect.second.translate(delta);
}

void LinkTag::draw(QPainter* painter)
{
	painter->save();
	if(m_hovering)
	{
		painter->setBrush(m_parentView->palette().highlight().color());
		QPen pen(m_parentView->palette().text().color());
		pen.setStyle(Qt::DotLine);
		painter->setPen(pen);

		for(auto tagRect : m_outputDatas)
			painter->drawLine(m_inputDataRect.center(), tagRect.second.center());
	}
	else
		painter->setBrush(m_parentView->palette().light());
	painter->setPen(QPen(m_parentView->palette().text().color()));

	QFont font;
	font.setPointSize(7);
	painter->setFont(font);

	auto indexText = QString::number(m_index + 1);

	// input
	qreal x = m_inputDataRect.left();
	qreal cy = m_inputDataRect.center().y();
	painter->drawLine(x - tagMargin, cy, x, cy);
	painter->drawRect(m_inputDataRect);
	painter->drawText(m_inputDataRect, Qt::AlignCenter, indexText);

	// outputs
	for(const auto& tagRectPair : m_outputDatas)
	{
		const auto& tagRect = tagRectPair.second;
		x = tagRect.right();
		cy = tagRect.center().y();
		painter->drawLine(x, cy, x + tagMargin, cy);
		painter->drawRect(tagRect);
		painter->drawText(tagRect, Qt::AlignCenter, indexText);
	}

	painter->restore();
}

bool LinkTag::needLinkTag(float inputX, float outputX)
{
	return outputX <= inputX;
}

std::vector<panda::BaseData*> LinkTag::getOutputDatas() const
{
	std::vector<panda::BaseData*> res;
	for (const auto dataRect : m_outputDatas)
		res.push_back(dataRect.first);
	return res;
}