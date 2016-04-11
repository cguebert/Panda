#include <ui/drawstruct/GenericObjectDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/object/GenericObject.h>
#include <panda/data/BaseData.h>
#include <panda/types/DataTraits.h>

#include <QPainter>

using panda::GenericObject;

GenericObjectDrawStruct::GenericObjectDrawStruct(GraphView* view, panda::GenericObject* object)
	: ObjectDrawStruct(view, (panda::PandaObject*)object)
	, m_genericObject(object)
	, m_nbDefInputs(0)
	, m_nbDefOutputs(0)
{
	update();

	for(GenericObject::GenericDataDefinition def : object->m_dataDefinitions)
	{
		if(def.input)
			++m_nbDefInputs;
		if(def.output)
			++m_nbDefOutputs;
	}
}

void GenericObjectDrawStruct::update()
{
	m_objectArea = QRectF(m_position + m_parentView->getViewDelta(), getObjectSize());

	m_datas.clear();
	std::vector<panda::BaseData*> inputDatas, outputDatas;
	inputDatas = m_object->getInputDatas();
	outputDatas = m_object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	int xi = m_objectArea.x() + dataRectMargin;
	int xo = m_objectArea.right() - dataRectMargin - dataRectSize;
	int startY = m_objectArea.y() + dataStartY();
	int dh = dataRectSize + dataRectMargin;
	int y;

	// First the "normal" datas
	int index = 0;
	for(int i=0; i<nbInputs; ++i)
	{
		panda::BaseData* data = inputDatas[i];
		if(!m_genericObject->m_createdDatasMap.count(data) && data != m_genericObject->m_genericData)
		{
			QRectF dataArea(xi, startY + index * dh,
							dataRectSize, dataRectSize);
			m_datas.emplace_back(dataArea, data);
			++index;
		}
	}
	y = startY + index * dh;

	index = 0;
	for(int i=0; i<nbOutputs; ++i)
	{
		panda::BaseData* data = outputDatas[i];
		if(!m_genericObject->m_createdDatasMap.count(data))
		{
			QRectF dataArea(xo, startY + index * dh,
							dataRectSize, dataRectSize);
			m_datas.emplace_back(dataArea, data);
			++index;
		}
	}
	y = qMax(y, startY + index * dh);

	// Now the created datas
	y += createdDataRectMargin;
	int nbCreated = m_genericObject->m_createdDatasStructs.size();
	int nbDef = m_genericObject->m_dataDefinitions.size();
	for(int i=0; i<nbCreated; ++i)
	{
		GenericObject::DataPtrList createdDatas = m_genericObject->m_createdDatasStructs[i]->datas;
		int inputIndex = 0, outputIndex = 0;
		for(int j=0; j<nbDef; ++j)
		{
			if(!createdDatas[j])
				continue;

			if(m_genericObject->m_dataDefinitions[j].input)
			{
				QRectF dataArea(xi, y + inputIndex * dh, dataRectSize, dataRectSize);
				m_datas.emplace_back(dataArea, createdDatas[j].get());
				++inputIndex;
			}
			if(m_genericObject->m_dataDefinitions[j].output)
			{
				QRectF dataArea(xo, y + outputIndex * dh, dataRectSize, dataRectSize);
				m_datas.emplace_back(dataArea, createdDatas[j].get());
				++outputIndex;
			}
		}
		y += qMax(inputIndex, outputIndex) * dh;
		y += createdDataRectMargin;
	}

	// And the generic data
	QRectF dataArea(xi, y, dataRectSize, dataRectSize);
	m_datas.emplace_back(dataArea, (panda::BaseData*)m_genericObject->m_genericData);
}

void GenericObjectDrawStruct::drawDatas(QPainter* painter)
{
	for(RectDataPair dataPair : m_datas)
	{
		if (dynamic_cast<panda::BaseGenericData*>(dataPair.second))
		{
			auto data = dataPair.second;
			painter->setPen(QPen(m_parentView->palette().text().color()));
			const panda::BaseData* clickedData = m_parentView->getClickedData();
			if (clickedData && clickedData != data && m_parentView->canLinkWith(data))
				painter->setBrush(QColor(clickedData->getDataTrait()->typeColor()));
			else
				painter->setBrush(m_parentView->palette().button().color());
			painter->drawRoundedRect(dataPair.first, 3, 3);
		}
		else
			drawData(painter, dataPair.second, dataPair.first);
	}
}

QSize GenericObjectDrawStruct::getObjectSize()
{
	QSize objectSize(objectDefaultWidth, objectDefaultHeight);

	int nbInputs, nbOutputs;
	nbInputs = m_object->getInputDatas().size();
	nbOutputs = m_object->getOutputDatas().size();

	int nbCreated = m_genericObject->m_createdDatasStructs.size();
	int nbCreatedInputs = nbCreated * m_nbDefInputs;
	int nbCreatedOutputs = nbCreated * m_nbDefOutputs;

	nbInputs -= nbCreatedInputs + 1; // (Removing the generic data too)
	nbOutputs -= nbCreatedOutputs;

	int maxData = qMax(nbInputs, nbOutputs);
	int normalDatasH = qMax(0,(maxData-1)*dataRectMargin + maxData*dataRectSize);
	if(maxData)
		normalDatasH += createdDataRectMargin;
	normalDatasH += dataRectMargin + dataRectSize;	// Adding the generic data now

	int maxCreatedDatas = qMax(m_nbDefInputs, m_nbDefOutputs);
	int createdDatasH = (maxCreatedDatas-1)*dataRectMargin + maxCreatedDatas*dataRectSize;
	int totalCreatedDatasH = nbCreated*(createdDataRectMargin + dataRectMargin + createdDatasH);

	objectSize.rheight() = qMax(objectSize.rheight(), 2*dataStartY() + normalDatasH + totalCreatedDatasH);

	return objectSize;
}

int GenericObjectDrawClass = RegisterDrawObject<panda::GenericObject, GenericObjectDrawStruct>();
