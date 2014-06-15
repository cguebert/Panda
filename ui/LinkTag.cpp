#include <QPainter>

#include <ui/GraphView.h>
#include <ui/LinkTag.h>

LinkTag::LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output)
	: index(0)
	, hovering(false)
	, parentView(view)
	, inputData(input)
{
	outputDatas.insert(output, QRectF());
}

void LinkTag::addOutput(panda::BaseData* output)
{
	if(!outputDatas.contains(output))
		outputDatas.insert(output, QRectF());
}

void LinkTag::removeOutput(panda::BaseData* output)
{
	outputDatas.remove(output);
}

void LinkTag::update()
{
	QRectF dataRect = parentView->getDataRect(inputData);
	inputDataRect = QRectF(dataRect.right() + tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);

	qreal ix = dataRect.center().x();

	QMutableMapIterator<panda::BaseData*, QRectF> iter(outputDatas);
	while(iter.hasNext())
	{
		iter.next();
		dataRect = parentView->getDataRect(iter.key());
		qreal ox = dataRect.center().x();
		if(ix < ox)
			iter.remove();
		else
		{
			QRectF tagRect(dataRect.left() - tagW - tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);
			iter.setValue(tagRect);
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

	for(auto rect : outputDatas.values())
		if(rect.contains(point))
			return true;

	return false;
}

void LinkTag::moveView(const QPointF& delta)
{
	inputDataRect.translate(delta);
	for(auto& rect : outputDatas)
		rect.translate(delta);
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

		for(auto tagRect : outputDatas.values())
			painter->drawLine(inputDataRect.center(), tagRect.center());
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
	for(auto tagRect : outputDatas.values())
	{
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
