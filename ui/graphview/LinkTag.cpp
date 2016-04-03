#include <QPainter>

#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTag.h>

LinkTag::LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output)
	: index(0)
	, hovering(false)
	, parentView(view)
	, inputData(input)
{
	outputDatas.emplace(output, QRectF());
}

void LinkTag::addOutput(panda::BaseData* output)
{
	if(!outputDatas.count(output))
		outputDatas.emplace(output, QRectF());
}

void LinkTag::removeOutput(panda::BaseData* output)
{
	outputDatas.erase(output);
}

void LinkTag::update()
{
	QRectF dataRect = parentView->getDataRect(inputData);
	inputDataRect = QRectF(dataRect.right() + tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);

	qreal ix = dataRect.center().x();

	for (auto it = outputDatas.begin(); it != outputDatas.end();)
	{
		dataRect = parentView->getDataRect(it->first);
		qreal ox = dataRect.center().x();
		if (ix < ox)
			it = outputDatas.erase(it);
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
	return outputDatas.empty();
}

bool LinkTag::isHovering(const QPointF& point)
{
	if(inputDataRect.contains(point))
		return true;

	for(auto rect : outputDatas)
		if(rect.second.contains(point))
			return true;

	return false;
}

void LinkTag::moveView(const QPointF& delta)
{
	inputDataRect.translate(delta);
	for(auto& rect : outputDatas)
		rect.second.translate(delta);
}

void LinkTag::draw(QPainter* painter)
{
	painter->save();
	if(hovering)
	{
		painter->setBrush(parentView->palette().highlight().color());
		QPen pen(parentView->palette().text().color());
		pen.setStyle(Qt::DotLine);
		painter->setPen(pen);

		for(auto tagRect : outputDatas)
			painter->drawLine(inputDataRect.center(), tagRect.second.center());
	}
	else
		painter->setBrush(parentView->palette().light());
	painter->setPen(QPen(parentView->palette().text().color()));

	QFont font;
	font.setPointSize(7);
	painter->setFont(font);

	// input
	qreal x = inputDataRect.left();
	qreal cy = inputDataRect.center().y();
	painter->drawLine(x - tagMargin, cy, x, cy);
	painter->drawRect(inputDataRect);
	painter->drawText(inputDataRect, Qt::AlignCenter, QString::number(index+1));

	// outputs
	for(const auto& tagRectPair : outputDatas)
	{
		const auto& tagRect = tagRectPair.second;
		x = tagRect.right();
		cy = tagRect.center().y();
		painter->drawLine(x, cy, x + tagMargin, cy);
		painter->drawRect(tagRect);
		painter->drawText(tagRect, Qt::AlignCenter, QString::number(index+1));
	}

	painter->restore();
}

panda::BaseData* LinkTag::getInputData()
{
	return inputData;
}
