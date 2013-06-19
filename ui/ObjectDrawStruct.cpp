#include <QtWidgets>

#include <ui/ObjectDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/PandaObject.h>
#include <panda/PandaDocument.h>

ObjectDrawStruct::ObjectDrawStruct(GraphView* view, panda::PandaObject* obj)
	: parentView(view), object(obj)
{
	QSize objSize = this->getObjectSize();
	QPointF center = view->contentsRect().center();
	position = QPointF(center.x() - objSize.width()/2, center.y() - objSize.height()/2) - view->getViewDelta();

	update();
}

void ObjectDrawStruct::update()
{
	objectArea = QRectF(position + parentView->getViewDelta(), this->getObjectSize());

	datas.clear();
	QList<panda::BaseData*> inputDatas, outputDatas;
	inputDatas = object->getInputDatas();
	outputDatas = object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	for(int i=0; i<nbInputs; ++i)
	{
		QRectF dataArea(objectArea.x() + dataRectMargin,
						objectArea.y() + dataStartY() + i * (dataRectSize + dataRectMargin),
						dataRectSize, dataRectSize);
		datas.append(qMakePair(dataArea, inputDatas[i]));
	}

	for(int i=0; i<nbOutputs; ++i)
	{
		QRectF dataArea(objectArea.right() - dataRectMargin - dataRectSize,
						objectArea.y() + dataStartY() + i * (dataRectSize + dataRectMargin),
						dataRectSize, dataRectSize);
		datas.append(qMakePair(dataArea, outputDatas[i]));
	}
}

void ObjectDrawStruct::move(const QPointF& delta)
{
	if(!delta.isNull())
	{
		position += delta;
		moveVisual(delta);
	}
}

void ObjectDrawStruct::moveVisual(const QPointF& delta)
{
	if(!delta.isNull())
	{
		objectArea.translate(delta);
		for(QList<RectDataPair>::iterator iter=datas.begin(); iter!=datas.end(); ++iter)
			iter->first.translate(delta);
	}
}

QSize ObjectDrawStruct::getObjectSize()
{
	QSize objectSize(objectDefaultWidth, objectDefaultHeight);

	int nbInputs, nbOutputs;
	nbInputs = object->getInputDatas().size();
	nbOutputs = object->getOutputDatas().size();
	int maxData = qMax(nbInputs, nbOutputs);
	objectSize.rheight() = qMax(objectSize.rheight(), 2*dataStartY() + (maxData-1)*dataRectMargin + maxData*dataRectSize);

	return objectSize;
}

panda::BaseData* ObjectDrawStruct::getDataAtPos(const QPointF& pt, QPointF* center)
{
	for(QList<RectDataPair>::iterator iter=datas.begin(); iter!=datas.end(); ++iter)
	{
		if(iter->first.contains(pt))
		{
			if(center)
				*center = iter->first.center();
			return iter->second;
		}
	}

	return nullptr;
}

bool ObjectDrawStruct::getDataRect(panda::BaseData* data, QRectF& rect)
{
	for(QList<RectDataPair>::iterator iter=datas.begin(); iter!=datas.end(); ++iter)
	{
		if(iter->second == data)
		{
			rect = iter->first;
			return true;
		}
	}

	return false;
}

void ObjectDrawStruct::draw(QPainter* painter)
{
	// Choose the pen and the brush
	QPen pen(parentView->palette().text().color());
	if(parentView->getDocument()->isSelected(object))
	{
		pen.setWidthF(3);
		painter->setBrush(parentView->palette().midlight());
	}
	else
		painter->setBrush(parentView->palette().light());
	painter->setPen(pen);

	// Draw the shape around the object
	drawShape(painter);

	// The Datas
	drawDatas(painter);

	// The Text
	drawText(painter);
}

void ObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawRoundedRect(objectArea, 5, 5);
}

void ObjectDrawStruct::drawDatas(QPainter* painter)
{
	painter->setPen(QPen(parentView->palette().text().color()));
	const panda::BaseData* clickedData = parentView->getClickedData();
	foreach(RectDataPair dataPair, datas)
	{
		if(clickedData
			&& clickedData != dataPair.second
			&& !GraphView::isCompatible(clickedData, dataPair.second))
			painter->setBrush(parentView->palette().dark());
		else
			painter->setBrush(parentView->palette().button());

		painter->drawRect(dataPair.first);
	}
}

void ObjectDrawStruct::drawText(QPainter* painter)
{
	painter->drawText(objectArea, Qt::AlignCenter, object->getName());
}

void ObjectDrawStruct::drawLinks(QPainter* painter)
{
	ObjectDrawStruct::RectDataIterator iterData = getDatasIterator();
	while(iterData.hasNext())
	{
		ObjectDrawStruct::RectDataPair rectData = iterData.next();
		panda::BaseData* data = rectData.second;
		panda::BaseData* parent = data->getParent();
		if(parent && !data->isOutput())
		{
			QRectF dataRect;
			if(parentView->getObjectDrawStruct(parent->getOwner())->getDataRect(parent, dataRect))
			{
				QPen pen(parentView->palette().text().color());
				painter->setPen(pen);
				QPointF d1 = dataRect.center(), d2 = rectData.first.center();
				if(d2.x()-d1.x() > 0)
				{
					double w = (d2.x()-d1.x()) / 2;
					QPainterPath path;
					path.moveTo(d1);
					path.cubicTo(d1+QPointF(w,0), d2-QPointF(w,0), d2);
					painter->drawPath(path);
				}
			}
		}
	}
}

bool ObjectDrawStruct::contains(const QPointF& point)
{
	return objectArea.contains(point);
}

QRectF ObjectDrawStruct::getObjectArea() const
{
	return objectArea;
}

QPointF ObjectDrawStruct::getPosition() const
{
	return position;
}

ObjectDrawStruct::RectDataIterator ObjectDrawStruct::getDatasIterator() const
{
	return RectDataIterator(datas);
}

int ObjectDrawStruct::dataStartY()
{
	return dataRectMargin;
}

//******************************************************************************************

ObjectDrawStructFactory* ObjectDrawStructFactory::getInstance()
{
	static ObjectDrawStructFactory factory;
	return &factory;
}

ObjectDrawStruct* ObjectDrawStructFactory::createDrawStruct(GraphView* view, panda::PandaObject* obj)
{
	foreach(QSharedPointer<BaseObjectDrawCreator> creator, creators)
	{
		if(creator->getClass()->isInstance(obj))
			return creator->create(view, obj);
	}

	return new ObjectDrawStruct(view, obj);
}

void ObjectDrawStructFactory::addCreator(BaseObjectDrawCreator* creator)
{
	const panda::BaseClass* newClass = creator->getClass();
	QSharedPointer<BaseObjectDrawCreator> ptr(creator);

	unsigned int nb = creators.size();
	for(unsigned int i=0; i<nb; ++i)
	{
		const panda::BaseClass* prevClass = creators[i]->getClass();
		if(newClass->hasParent(prevClass))
		{
			creators.insert(i, ptr);
			return;
		}
	}

	creators.append(ptr);
}
