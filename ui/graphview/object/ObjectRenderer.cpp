#include <ui/graphview/object/ObjectRenderer.h>

#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinksList.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ViewInteraction.h>
#include <ui/graphview/object/ObjectPositionAddon.h>

#include <panda/object/PandaObject.h>
#include <panda/object/ObjectAddons.h>
#include <panda/messaging.h>
#include <panda/document/PandaDocument.h>
#include <panda/types/DataTraits.h>

#include <cmath>

using panda::types::Point;
using panda::types::Rect;

namespace graphview
{

namespace object
{

ObjectRenderer::ObjectRenderer(GraphView* view, panda::PandaObject* obj)
	: m_parentView(view), m_object(obj)
	, m_positionAddon(obj->addons().edit<ObjectPositionAddon>())
	, m_observer(std::make_unique<panda::msg::Observer>())
{
	if (!m_positionAddon.isSet())
	{
		Point objSize = getObjectSize();
		Point center = view->getNewObjectPosition();
		m_position = (center - objSize / 2);
		m_positionAddon.setPosition((center - objSize / 2));
	}
	else
		m_position = m_positionAddon.getPosition();

	m_observer->get(m_positionAddon.positionChanged).connect<ObjectRenderer, &ObjectRenderer::positionChanged>(this);
}

ObjectRenderer::~ObjectRenderer()
{
}

void ObjectRenderer::initializeRenderer(ViewRenderer& viewRenderer)
{
	m_textDrawList = std::make_unique<graphics::DrawList>(viewRenderer);
}

void ObjectRenderer::update()
{
	m_selectionArea = m_visualArea = Rect::fromSize(getPosition(), getObjectSize());

	m_datas.clear();
	std::vector<panda::BaseData*> inputDatas, outputDatas;
	inputDatas = m_object->getInputDatas();
	outputDatas = m_object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	for(int i=0; i<nbInputs; ++i)
	{
		Rect dataArea = Rect::fromSize(m_visualArea.left() + dataRectMargin,
									   m_visualArea.top() + dataStartY() + i * (dataRectSize + dataRectMargin),
									   dataRectSize, dataRectSize);
		m_datas.emplace_back(inputDatas[i], dataArea);
	}

	for(int i=0; i<nbOutputs; ++i)
	{
		Rect dataArea = Rect::fromSize(m_visualArea.right() - dataRectMargin - dataRectSize,
									   m_visualArea.top() + dataStartY() + i * (dataRectSize + dataRectMargin),
									   dataRectSize, dataRectSize);
		m_datas.emplace_back(outputDatas[i], dataArea);
	}

	createShape();
}

void ObjectRenderer::createShape()
{
	m_outline.clear();
	m_outline.rect(m_visualArea, objectCorner);
	m_outline.close();
	m_fillShape = m_outline.triangulate();
}

void ObjectRenderer::move(const Point& delta)
{
	if(!delta.isNull())
	{
		m_position += delta;
		if (m_positionAddon.getPosition() != m_position)
			m_positionAddon.setPosition(m_position);

		m_visualArea.translate(delta);
		m_selectionArea.translate(delta);
		for (auto& it : m_datas)
			it.second.translate(delta);
		m_outline.translate(delta);
		m_fillShape.translate(delta);
		m_textDrawList->translate(delta);
	}
}

Point ObjectRenderer::getObjectSize()
{
	Point objectSize(objectDefaultWidth, objectDefaultHeight);

	int nbInputs, nbOutputs;
	nbInputs = m_object->getInputDatas().size();
	nbOutputs = m_object->getOutputDatas().size();
	int maxData = std::max(nbInputs, nbOutputs);
	objectSize.y = std::max(objectSize.y, 2.0f * dataStartY() + (maxData-1)*dataRectMargin + maxData*dataRectSize);

	return objectSize;
}

Rect ObjectRenderer::getTextArea()
{
	int margin = dataRectSize + dataRectMargin + 3;
	return m_visualArea.adjusted(margin, 0, -margin, 0);
}

panda::BaseData* ObjectRenderer::getDataAtPos(const Point& pt, Point* center) const
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

bool ObjectRenderer::getDataRect(const panda::BaseData* data, Rect& rect) const
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

void ObjectRenderer::draw(graphics::DrawList& list, graphics::DrawColors& colors, bool selected)
{
	colors.penWidth = selected ? 3.f : 1.f;
	colors.fillColor = selected ? colors.midLightColor : graphics::DrawList::setAlpha(colors.lightColor, 128);

	// Draw the shape around the object
	drawShape(list, colors);

	// The Datas
	drawDatas(list, colors);

	// The Text
	drawText(list, colors);
}

void ObjectRenderer::drawShape(graphics::DrawList& list, graphics::DrawColors& colors)
{
	// Draw the shape around the object
	list.addMesh(m_fillShape, colors.fillColor);
	list.addPolyline(m_outline, colors.penColor, false, colors.penWidth);
}

void ObjectRenderer::drawDatas(graphics::DrawList& list, graphics::DrawColors& colors)
{
	for(const auto& dataPair : m_datas)
		drawData(list, colors, dataPair.first, dataPair.second);
}

void ObjectRenderer::drawData(graphics::DrawList& list, graphics::DrawColors& colors, const panda::BaseData* data, const Rect& area)
{
	unsigned int dataCol = 0;
	const panda::BaseData* clickedData = getParentView()->interaction().clickedData();
	if (clickedData && clickedData != data && !getParentView()->linksList().canLinkWith(data))
		dataCol = colors.lightColor;
	else
		dataCol = graphics::DrawList::convert(data->getDataTrait()->typeColor()) | 0xFF000000; // We have to set the alpha

	list.addRectFilled(area, dataCol);
	list.addRect(area, colors.penColor);
}

void ObjectRenderer::drawText(graphics::DrawList& list, graphics::DrawColors& colors)
{
	auto label = getLabel();
	if (label != m_currentLabel)
	{
		m_currentLabel = label;
		m_textDrawList->clear();
		m_textDrawList->addText(getTextArea(), label, colors.penColor, graphics::DrawList::Align_Center, 1.0f, true, true);
	}
	
	list.merge(*m_textDrawList);
}

std::string ObjectRenderer::getLabel() const
{
	const auto& name = m_object->getName();
	const auto& label = m_object->getLabel();
	if (!label.empty())
		return label + "\n(" + name + ")";

	return name;
}

void ObjectRenderer::positionChanged(panda::types::Point newPos)
{
	if (m_position != newPos)
	{
		move(newPos - m_position);
		getParentView()->objectsMoved();
	}
}

bool ObjectRenderer::updateIfDirty()
{
	if (m_dirty)
	{
		m_dirty = false;
		update();
		return true;
	}

	return false;
}

void ObjectRenderer::setDirty()
{
	m_dirty = true;
}

//****************************************************************************//

ObjectRendererFactory* ObjectRendererFactory::getInstance()
{
	static ObjectRendererFactory factory;
	return &factory;
}

std::shared_ptr<ObjectRenderer> ObjectRendererFactory::createRenderer(GraphView* view, panda::PandaObject* obj)
{
	for(const auto& creator : creators)
	{
		if(creator->getClass()->isInstance(obj))
			return creator->create(view, obj);
	}

	return std::make_shared<ObjectRenderer>(view, obj);
}

void ObjectRendererFactory::addCreator(BaseObjectDrawCreator* creator)
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

} // namespace object

} // namespace graphview
