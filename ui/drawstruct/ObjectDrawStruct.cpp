#include <QtWidgets>

#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/PandaObject.h>
#include <panda/PandaDocument.h>

ObjectDrawStruct::ObjectDrawStruct(GraphView* view, panda::PandaObject* obj)
	: m_parentView(view), m_object(obj)
{
	QSize objSize = getObjectSize();
	QPointF center = view->contentsRect().center();
	m_position = QPointF(center.x() - objSize.width()/2, center.y() - objSize.height()/2) - view->getViewDelta();

	update();
}

void ObjectDrawStruct::update()
{
	m_objectArea = QRectF(m_position + m_parentView->getViewDelta(), getObjectSize());

	m_datas.clear();
	std::vector<panda::BaseData*> inputDatas, outputDatas;
	inputDatas = m_object->getInputDatas();
	outputDatas = m_object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	for(int i=0; i<nbInputs; ++i)
	{
		QRectF dataArea(m_objectArea.x() + dataRectMargin,
						m_objectArea.y() + dataStartY() + i * (dataRectSize + dataRectMargin),
						dataRectSize, dataRectSize);
		m_datas.push_back(qMakePair(dataArea, inputDatas[i]));
	}

	for(int i=0; i<nbOutputs; ++i)
	{
		QRectF dataArea(m_objectArea.right() - dataRectMargin - dataRectSize,
						m_objectArea.y() + dataStartY() + i * (dataRectSize + dataRectMargin),
						dataRectSize, dataRectSize);
		m_datas.push_back(qMakePair(dataArea, outputDatas[i]));
	}
}

void ObjectDrawStruct::move(const QPointF& delta)
{
	if(!delta.isNull())
	{
		m_position += delta;
		moveVisual(delta);
	}
}

void ObjectDrawStruct::moveVisual(const QPointF& delta)
{
	if(!delta.isNull())
	{
		m_objectArea.translate(delta);
		for(std::vector<RectDataPair>::iterator iter=m_datas.begin(); iter!=m_datas.end(); ++iter)
			iter->first.translate(delta);
	}
}

QSize ObjectDrawStruct::getObjectSize()
{
	QSize objectSize(objectDefaultWidth, objectDefaultHeight);

	int nbInputs, nbOutputs;
	nbInputs = m_object->getInputDatas().size();
	nbOutputs = m_object->getOutputDatas().size();
	int maxData = qMax(nbInputs, nbOutputs);
	objectSize.rheight() = qMax(objectSize.rheight(), 2*dataStartY() + (maxData-1)*dataRectMargin + maxData*dataRectSize);

	return objectSize;
}

panda::BaseData* ObjectDrawStruct::getDataAtPos(const QPointF& pt, QPointF* center)
{
	for(std::vector<RectDataPair>::iterator iter=m_datas.begin(); iter!=m_datas.end(); ++iter)
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

bool ObjectDrawStruct::getDataRect(const panda::BaseData* data, QRectF& rect)
{
	for(std::vector<RectDataPair>::iterator iter=m_datas.begin(); iter!=m_datas.end(); ++iter)
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
	QPen pen(m_parentView->palette().text().color());
	if(m_parentView->getDocument()->isSelected(m_object))
	{
		pen.setWidthF(3);
		painter->setBrush(m_parentView->palette().midlight());
	}
	else
		painter->setBrush(m_parentView->palette().light());
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
	painter->drawRoundedRect(m_objectArea, 5, 5);
}

void ObjectDrawStruct::drawDatas(QPainter* painter)
{
	painter->setPen(QPen(m_parentView->palette().text().color()));
	const panda::BaseData* clickedData = m_parentView->getClickedData();
	for(RectDataPair dataPair : m_datas)
	{
		if(clickedData
			&& clickedData != dataPair.second
			&& !GraphView::isCompatible(clickedData, dataPair.second))
			painter->setBrush(m_parentView->palette().dark());
		else
			painter->setBrush(m_parentView->palette().button());

		painter->drawRect(dataPair.first);
	}
}

void ObjectDrawStruct::drawText(QPainter* painter)
{
	int margin = dataRectSize+dataRectMargin+3;
	QRectF textArea = m_objectArea.adjusted(margin, 0, -margin, 0);
	painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, m_object->getName());
}

void ObjectDrawStruct::drawLinks(QPainter* painter)
{
	for(const auto& rectData : m_datas)
	{
		panda::BaseData* data = rectData.second;
		panda::BaseData* parent = data->getParent();
		if(parent && !data->isOutput())
		{
			QRectF dataRect;
			if(m_parentView->getObjectDrawStruct(parent->getOwner())->getDataRect(parent, dataRect))
			{
				QPen pen(m_parentView->palette().text().color());
				pen.setWidth(1);
				painter->setPen(pen);
				QPointF d1 = dataRect.center(), d2 = rectData.first.center();
				if(d2.x()-d1.x() > 0) // We don't draw a link if it goes from right to left (see the LinkTag class)
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

void ObjectDrawStruct::save(QDomDocument&, QDomElement& elem)
{
	elem.setAttribute("x", m_position.x());
	elem.setAttribute("y", m_position.y());
}

void ObjectDrawStruct::load(QDomElement& elem)
{
	QPointF newPos;
#ifdef PANDA_DOUBLE
	newPos.setX(elem.attribute("x").toDouble());
	newPos.setY(elem.attribute("y").toDouble());
#else
	newPos.setX(elem.attribute("x").toFloat());
	newPos.setY(elem.attribute("y").toFloat());
#endif
	move(newPos - m_position);
}

//****************************************************************************//

ObjectDrawStructFactory* ObjectDrawStructFactory::getInstance()
{
	static ObjectDrawStructFactory factory;
	return &factory;
}

ObjectDrawStruct* ObjectDrawStructFactory::createDrawStruct(GraphView* view, panda::PandaObject* obj)
{
	for(const auto& creator : creators)
	{
		if(creator->getClass()->isInstance(obj))
			return creator->create(view, obj);
	}

	return new ObjectDrawStruct(view, obj);
}

void ObjectDrawStructFactory::addCreator(BaseObjectDrawCreator* creator)
{
	const panda::BaseClass* newClass = creator->getClass();
	std::shared_ptr<BaseObjectDrawCreator> ptr(creator);

	unsigned int nb = creators.size();
	for(unsigned int i=0; i<nb; ++i)
	{
		const panda::BaseClass* prevClass = creators[i]->getClass();
		if(newClass->hasParent(prevClass))
		{
			creators.insert(creators.begin() + i, ptr);
			return;
		}
	}

	creators.push_back(ptr);
}
