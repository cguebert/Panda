#include <panda/graphview/object/GenericObjectRenderer.h>
#include <panda/graphview/object/ObjectRendererFactory.h>

#include <panda/graphview/GraphView.h>
#include <panda/graphview/LinksList.h>
#include <panda/graphview/ViewInteraction.h>

#include <panda/object/GenericObject.h>
#include <panda/data/BaseData.h>
#include <panda/types/DataTraits.h>

namespace panda
{

using types::Point;
using types::Rect;

namespace graphview
{

namespace object
{

GenericObjectRenderer::GenericObjectRenderer(GraphView* view, GenericObject* object)
	: ObjectRenderer(view, (PandaObject*)object)
	, m_genericObject(object)
	, m_nbDefInputs(0)
	, m_nbDefOutputs(0)
{
	for (GenericObject::GenericDataDefinition def : object->m_dataDefinitions)
	{
		if (def.isInput())
			++m_nbDefInputs;
		if (def.isOutput())
			++m_nbDefOutputs;
	}
}

void GenericObjectRenderer::update()
{
	m_selectionArea = m_visualArea = Rect::fromSize(getPosition(), getObjectSize());

	m_datas.clear();
	std::vector<BaseData*> inputDatas, outputDatas;
	inputDatas = m_object->getInputDatas();
	outputDatas = m_object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	float xi = m_visualArea.left() + dataRectMargin;
	float xo = m_visualArea.right() - dataRectMargin - dataRectSize;
	float startY = m_visualArea.top() + dataStartY();
	float dh = dataRectSize + dataRectMargin;
	float drs = dataRectSize;
	float y;

	// First the "normal" datas
	int index = 0;
	for (int i = 0; i < nbInputs; ++i)
	{
		BaseData* data = inputDatas[i];
		if (!m_genericObject->m_createdDatasMap.count(data) && data != m_genericObject->m_genericData)
		{
			Rect dataArea = Rect::fromSize(xi, startY + index * dh, drs, drs);
			m_datas.emplace_back(data, dataArea);
			++index;
		}
	}
	y = startY + index * dh;

	index = 0;
	for (int i = 0; i < nbOutputs; ++i)
	{
		BaseData* data = outputDatas[i];
		if (!m_genericObject->m_createdDatasMap.count(data))
		{
			Rect dataArea = Rect::fromSize(xo, startY + index * dh, drs, drs);
			m_datas.emplace_back(data, dataArea);
			++index;
		}
	}
	y = std::max(y, startY + index * dh);

	// Now the created datas
	y += createdDataRectMargin;
	int nbCreated = m_genericObject->m_createdDatasStructs.size();
	int nbDef = m_genericObject->m_dataDefinitions.size();
	for (int i = 0; i < nbCreated; ++i)
	{
		GenericObject::DataPtrList createdDatas = m_genericObject->m_createdDatasStructs[i]->datas;
		int inputIndex = 0, outputIndex = 0;
		for (int j = 0; j < nbDef; ++j)
		{
			if (!createdDatas[j])
				continue;

			if (m_genericObject->m_dataDefinitions[j].isInput())
			{
				Rect dataArea = Rect::fromSize(xi, y + inputIndex * dh, drs, drs);
				m_datas.emplace_back(createdDatas[j].get(), dataArea);
				++inputIndex;
			}
			if (m_genericObject->m_dataDefinitions[j].isOutput())
			{
				Rect dataArea = Rect::fromSize(xo, y + outputIndex * dh, drs, drs);
				m_datas.emplace_back(createdDatas[j].get(), dataArea);
				++outputIndex;
			}
		}
		y += std::max(inputIndex, outputIndex) * dh;
		y += createdDataRectMargin;
	}

	// And the generic data
	Rect dataArea = Rect::fromSize(xi, y, drs, drs);
	m_datas.emplace_back((BaseData*)m_genericObject->m_genericData, dataArea);

	createShape();
}

void GenericObjectRenderer::drawDatas(graphics::DrawList& list, graphics::DrawColors& colors)
{
	const BaseData* clickedData = getParentView()->interaction().clickedData();

	for (DataRectPair dataPair : m_datas)
	{
		auto data = dataPair.first;
		if (dynamic_cast<BaseGenericData*>(data))
		{
			unsigned int dataCol = 0;
			if (clickedData && clickedData != data && getParentView()->linksList().canLinkWith(data))
				dataCol = graphics::DrawList::convert(clickedData->getDataTrait()->typeColor()) | 0xFF000000; // We have to set the alpha
			else
				dataCol = colors.lightColor;

			const auto& area = dataPair.second;
			list.addRectFilled(area, dataCol, 3.0f);
			list.addRect(area, colors.penColor, 1.0f, 3.0f);
		}
		else
			drawData(list, colors, dataPair.first, dataPair.second);
	}
}

Point GenericObjectRenderer::getObjectSize()
{
	Point objectSize(static_cast<float>(objectDefaultWidth), static_cast<float>(objectDefaultHeight));

	int nbInputs, nbOutputs;
	nbInputs = m_object->getInputDatas().size();
	nbOutputs = m_object->getOutputDatas().size();

	int nbCreated = m_genericObject->m_createdDatasStructs.size();
	int nbCreatedInputs = nbCreated * m_nbDefInputs;
	int nbCreatedOutputs = nbCreated * m_nbDefOutputs;

	nbInputs -= nbCreatedInputs + 1; // (Removing the generic data too)
	nbOutputs -= nbCreatedOutputs;

	int maxData = std::max(nbInputs, nbOutputs);
	int normalDatasH = std::max(0, (maxData - 1)*dataRectMargin + maxData*dataRectSize);
	if (maxData)
		normalDatasH += createdDataRectMargin;
	normalDatasH += dataRectMargin + dataRectSize;	// Adding the generic data now

	int maxCreatedDatas = std::max(m_nbDefInputs, m_nbDefOutputs);
	int createdDatasH = (maxCreatedDatas - 1)*dataRectMargin + maxCreatedDatas*dataRectSize;
	int totalCreatedDatasH = nbCreated*(createdDataRectMargin + dataRectMargin + createdDatasH);

	objectSize.y = std::max(objectSize.y, 2.0f * dataStartY() + normalDatasH + totalCreatedDatasH);

	return objectSize;
}

int GenericObjectDrawClass = RegisterDrawObject<GenericObject, GenericObjectRenderer>();

} // namespace object

} // namespace graphview

} // namespace panda
