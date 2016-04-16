#include <QtWidgets>

#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/ObjectsSelection.h>

#include <panda/object/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/types/DataTraits.h>

#include <cmath>

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
		m_datas.emplace_back(dataArea, inputDatas[i]);
	}

	for(int i=0; i<nbOutputs; ++i)
	{
		QRectF dataArea(m_objectArea.right() - dataRectMargin - dataRectSize,
						m_objectArea.y() + dataStartY() + i * (dataRectSize + dataRectMargin),
						dataRectSize, dataRectSize);
		m_datas.emplace_back(dataArea, outputDatas[i]);
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
	objectSize.rheight() = qMax(objectSize.height(), 2*dataStartY() + (maxData-1)*dataRectMargin + maxData*dataRectSize);

	return objectSize;
}

QRectF ObjectDrawStruct::getTextArea()
{
	int margin = dataRectSize + dataRectMargin + 3;
	return QRectF(m_objectArea.adjusted(margin, 0, -margin, 0));
}

panda::BaseData* ObjectDrawStruct::getDataAtPos(const QPointF& pt, QPointF* center) const
{
	for(const auto& iter : m_datas)
	{
		if(iter.first.contains(pt))
		{
			if(center)
				*center = iter.first.center();
			return iter.second;
		}
	}

	return nullptr;
}

bool ObjectDrawStruct::getDataRect(const panda::BaseData* data, QRectF& rect) const
{
	for(const auto& iter : m_datas)
	{
		if(iter.second == data)
		{
			rect = iter.first;
			return true;
		}
	}

	return false;
}

void ObjectDrawStruct::fillDrawList(DrawList& list, bool selected)
{
	const auto& palette = m_parentView->palette();
	unsigned int penCol = DrawList::convert(palette.text().color());
	unsigned int midCol = DrawList::convert(palette.midlight().color());
	unsigned int lightCol = DrawList::convert(palette.light().color());
	float penWidth = selected ? 3.f : 1.f;
	unsigned int fillCol = selected ? midCol : DrawList::setAlpha(lightCol, 128);

	// Draw the shape around the object
	pPoint tl = pPoint(m_objectArea.left(), m_objectArea.top()), br = pPoint(m_objectArea.right(), m_objectArea.bottom());
	list.addRectFilled(tl, br, fillCol, objectCorner);
	list.addRect(tl, br, penCol, penWidth, objectCorner);

	// The Datas
	const panda::BaseData* clickedData = m_parentView->getClickedData();
	for (const auto& dataPair : m_datas)
	{
		const auto data = dataPair.second;
		const auto area = dataPair.first;
		unsigned int dataCol = 0;
		if (clickedData && clickedData != data && !m_parentView->canLinkWith(data))
			dataCol = lightCol;
		else
			dataCol = DrawList::setAlpha(DrawList::convert(data->getDataTrait()->typeColor()), 255);

		pPoint dtl = pPoint(area.left(), area.top()), dbr = pPoint(area.right(), area.bottom());
		list.addRectFilled(dtl, dbr, dataCol);
		list.addRect(dtl, dbr, penCol);
	}
	
	// The Text
	QRectF textArea = getTextArea();
	pRect pArea(textArea.left(), textArea.top(), textArea.right(), textArea.bottom());
	list.addText(pArea, penCol, m_object->getName(), DrawList::Align_Center | DrawList::Align_VCenter);
}

void ObjectDrawStruct::draw(QPainter* painter, bool selected)
{
	// Choose the pen and the brush
	const auto& palette = m_parentView->palette();
	QPen pen(palette.text().color());
	if(selected)
	{
		pen.setWidthF(3);
		painter->setBrush(palette.midlight());
	}
	else
	{
		auto color = palette.light().color();
		color.setAlpha(128);
		painter->setBrush(color);
	}

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
	painter->drawRoundedRect(m_objectArea, objectCorner, objectCorner);
}

void ObjectDrawStruct::drawDatas(QPainter* painter)
{
	for(const auto& dataPair : m_datas)
		drawData(painter, dataPair.second, dataPair.first);
}

void ObjectDrawStruct::drawData(QPainter* painter, const panda::BaseData* data, const QRectF& area)
{
	painter->setPen(QPen(m_parentView->palette().text().color()));
	const panda::BaseData* clickedData = m_parentView->getClickedData();

	if (clickedData && clickedData != data && !m_parentView->canLinkWith(data))
		painter->setBrush(m_parentView->palette().light());
	else
		painter->setBrush(QColor(data->getDataTrait()->typeColor()));

	painter->drawRect(area);
}

void ObjectDrawStruct::drawText(QPainter* painter)
{
	QRectF textArea = getTextArea();
	painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, QString::fromStdString(m_object->getName()));
}

void ObjectDrawStruct::save(panda::XmlElement& elem)
{
	elem.setAttribute("x", m_position.x());
	elem.setAttribute("y", m_position.y());
}

void ObjectDrawStruct::load(const panda::XmlElement& elem)
{
	QPointF newPos;
	newPos.setX(elem.attribute("x").toFloat());
	newPos.setY(elem.attribute("y").toFloat());
	move(newPos - m_position);
}

//****************************************************************************//

ObjectDrawStructFactory* ObjectDrawStructFactory::getInstance()
{
	static ObjectDrawStructFactory factory;
	return &factory;
}

std::shared_ptr<ObjectDrawStruct> ObjectDrawStructFactory::createDrawStruct(GraphView* view, panda::PandaObject* obj)
{
	for(const auto& creator : creators)
	{
		if(creator->getClass()->isInstance(obj))
			return creator->create(view, obj);
	}

	return std::make_shared<ObjectDrawStruct>(view, obj);
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
