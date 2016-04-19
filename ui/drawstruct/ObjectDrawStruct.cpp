#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/ObjectsSelection.h>

#include <panda/object/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/types/DataTraits.h>

#include <cmath>

using panda::types::Point;
using panda::types::Rect;

ObjectDrawStruct::ObjectDrawStruct(GraphView* view, panda::PandaObject* obj)
	: m_parentView(view), m_object(obj)
{
	Point objSize = getObjectSize();
	Point center = view->getNewObjectPosition();
	m_position = (center - objSize/2);

	update();
}

void ObjectDrawStruct::update()
{
	m_objectArea = Rect::fromSize(m_position, getObjectSize());

	m_datas.clear();
	std::vector<panda::BaseData*> inputDatas, outputDatas;
	inputDatas = m_object->getInputDatas();
	outputDatas = m_object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	for(int i=0; i<nbInputs; ++i)
	{
		Rect dataArea = Rect::fromSize(m_objectArea.left() + dataRectMargin,
									   m_objectArea.top() + dataStartY() + i * (dataRectSize + dataRectMargin),
									   dataRectSize, dataRectSize);
		m_datas.emplace_back(inputDatas[i], dataArea);
	}

	for(int i=0; i<nbOutputs; ++i)
	{
		Rect dataArea = Rect::fromSize(m_objectArea.right() - dataRectMargin - dataRectSize,
									   m_objectArea.top() + dataStartY() + i * (dataRectSize + dataRectMargin),
									   dataRectSize, dataRectSize);
		m_datas.emplace_back(outputDatas[i], dataArea);
	}

	createShape();
}

void ObjectDrawStruct::createShape()
{
	m_outline.clear();
	m_outline.rect(m_objectArea, objectCorner);
	m_outline.close();
	m_fillShape = m_outline.triangulate();
}

void ObjectDrawStruct::move(const Point& delta)
{
	if(!delta.isNull())
	{
		m_position += delta;
		m_objectArea.translate(delta);
		for (auto& it : m_datas)
			it.second.translate(delta);
		m_outline.translate(delta);
		m_fillShape.translate(delta);
		m_textDrawList.translate(delta);
	}
}

Point ObjectDrawStruct::getObjectSize()
{
	Point objectSize(objectDefaultWidth, objectDefaultHeight);

	int nbInputs, nbOutputs;
	nbInputs = m_object->getInputDatas().size();
	nbOutputs = m_object->getOutputDatas().size();
	int maxData = std::max(nbInputs, nbOutputs);
	objectSize.y = std::max(objectSize.y, 2.0f * dataStartY() + (maxData-1)*dataRectMargin + maxData*dataRectSize);

	return objectSize;
}

Rect ObjectDrawStruct::getTextArea()
{
	int margin = dataRectSize + dataRectMargin + 3;
	return m_objectArea.adjusted(margin, 0, -margin, 0);
}

panda::BaseData* ObjectDrawStruct::getDataAtPos(const Point& pt, Point* center) const
{
	for(const auto& iter : m_datas)
	{
		if(iter.second.contains(pt))
		{
			if(center)
				*center = iter.second.center();
			return iter.first;
		}
	}

	return nullptr;
}

bool ObjectDrawStruct::getDataRect(const panda::BaseData* data, Rect& rect) const
{
	auto it = std::find_if(m_datas.begin(), m_datas.end(), [data](const DataRectPair& p) {
		return p.first == data;
	});
	if (it != m_datas.end())
	{
		rect = it->second;
		return true;
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
	list.addMesh(m_fillShape, colors.fillColor);
	list.addPolyline(m_outline, colors.penColor, false, colors.penWidth);
}

void ObjectDrawStruct::drawDatas(DrawList& list, DrawColors& colors)
{
	for(const auto& dataPair : m_datas)
		drawData(list, colors, dataPair.first, dataPair.second);
}

void ObjectDrawStruct::drawData(DrawList& list, DrawColors& colors, const panda::BaseData* data, const Rect& area)
{
	unsigned int dataCol = 0;
	const panda::BaseData* clickedData = m_parentView->getClickedData();
	if (clickedData && clickedData != data && !m_parentView->canLinkWith(data))
		dataCol = colors.lightColor;
	else
		dataCol = DrawList::convert(data->getDataTrait()->typeColor()) | 0xFF000000; // We have to set the alpha

	list.addRectFilled(area, dataCol);
	list.addRect(area, colors.penColor);
}

void ObjectDrawStruct::drawText(DrawList& list, DrawColors& colors)
{
	auto label = getLabel();
	if (label != m_currentLabel)
	{
		m_currentLabel = label;
		m_textDrawList = DrawList();
		m_textDrawList.addText(getTextArea(), label, colors.penColor, DrawList::Align_Center);
	}
	
	list.merge(m_textDrawList);
}

void ObjectDrawStruct::save(panda::XmlElement& elem)
{
	elem.setAttribute("x", m_position.x);
	elem.setAttribute("y", m_position.y);
}

void ObjectDrawStruct::load(const panda::XmlElement& elem)
{
	Point newPos;
	newPos.x = elem.attribute("x").toFloat();
	newPos.y = elem.attribute("y").toFloat();
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
