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

void ObjectDrawStruct::draw(DrawList& list, DrawColors& colors, bool selected)
{
	colors.penWidth = selected ? 3.f : 1.f;
	colors.fillColor = selected ? colors.midLightColor : DrawList::setAlpha(colors.lightColor, 128);

	// Draw the shape around the object
	drawShape(list, colors);

	// The Datas
	drawDatas(list, colors);

	// The Text
	drawText(list, colors);
}

void ObjectDrawStruct::drawShape(DrawList& list, DrawColors& colors)
{
	// Draw the shape around the object
	pPoint tl = pPoint(m_objectArea.left(), m_objectArea.top()), br = pPoint(m_objectArea.right(), m_objectArea.bottom());
	list.addRectFilled(tl, br, colors.fillColor, objectCorner);
	list.addRect(tl, br, colors.penColor, colors.penWidth, objectCorner);
}

void ObjectDrawStruct::drawDatas(DrawList& list, DrawColors& colors)
{
	for(const auto& dataPair : m_datas)
		drawData(list, colors, dataPair.second, dataPair.first);
}

void ObjectDrawStruct::drawData(DrawList& list, DrawColors& colors, const panda::BaseData* data, const QRectF& area)
{
	unsigned int dataCol = 0;
	const panda::BaseData* clickedData = m_parentView->getClickedData();
	if (clickedData && clickedData != data && !m_parentView->canLinkWith(data))
		dataCol = colors.lightColor;
	else
		dataCol = DrawList::convert(data->getDataTrait()->typeColor()) | 0xFF000000; // We have to set the alpha

	pPoint dtl = pPoint(area.left(), area.top()), dbr = pPoint(area.right(), area.bottom());
	list.addRectFilled(dtl, dbr, dataCol);
	list.addRect(dtl, dbr, colors.penColor);
}

void ObjectDrawStruct::drawText(DrawList& list, DrawColors& colors)
{
	QRectF textArea = getTextArea();
	pRect pArea(textArea.left(), textArea.top(), textArea.right(), textArea.bottom());
	unsigned int penCol = DrawList::convert(m_parentView->palette().text().color());
	list.addText(pArea, getLabel(), penCol, DrawList::Align_Center);
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

std::string ObjectDrawStruct::getLabel() const
{
	return m_object->getName();
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
