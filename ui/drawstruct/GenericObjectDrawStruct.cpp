#include <ui/drawstruct/GenericObjectDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/GenericObject.h>

#include <QPainter>

using panda::GenericObject;

GenericObjectDrawStruct::GenericObjectDrawStruct(GraphView* view, panda::GenericObject* object)
	: ObjectDrawStruct(view, (panda::PandaObject*)object)
	, genericObject(object)
	, nbDefInputs(0)
	, nbDefOutputs(0)
{
	update();

	for(GenericObject::GenericDataDefinition def : object->m_dataDefinitions)
	{
		if(def.input)
			++nbDefInputs;
		if(def.output)
			++nbDefOutputs;
	}
}

void GenericObjectDrawStruct::update()
{
	m_objectArea = QRectF(m_position + m_parentView->getViewDelta(), getObjectSize());

	m_datas.clear();
	QList<panda::BaseData*> inputDatas, outputDatas;
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
		if(!genericObject->m_createdDatasMap.contains(data) && data != genericObject->m_genericData)
		{
			QRectF dataArea(xi, startY + index * dh,
							dataRectSize, dataRectSize);
			m_datas.append(qMakePair(dataArea, data));
			++index;
		}
	}
	y = startY + index * dh;

	index = 0;
	for(int i=0; i<nbOutputs; ++i)
	{
		panda::BaseData* data = outputDatas[i];
		if(!genericObject->m_createdDatasMap.contains(data))
		{
			QRectF dataArea(xo, startY + index * dh,
							dataRectSize, dataRectSize);
			m_datas.append(qMakePair(dataArea, data));
			++index;
		}
	}
	y = qMax(y, startY + index * dh);

	// Now the created datas
	y += createdDataRectMargin;
	int nbCreated = genericObject->m_createdDatasStructs.size();
	int nbDef = genericObject->m_dataDefinitions.size();
	for(int i=0; i<nbCreated; ++i)
	{
		GenericObject::DataPtrList createdDatas = genericObject->m_createdDatasStructs[i]->datas;
		int inputIndex = 0, outputIndex = 0;
		for(int j=0; j<nbDef; ++j)
		{
			if(!createdDatas[j])
				continue;

			if(genericObject->m_dataDefinitions[j].input)
			{
				QRectF dataArea(xi, y + inputIndex * dh, dataRectSize, dataRectSize);
				m_datas.append(qMakePair(dataArea, createdDatas[j].data()));
				++inputIndex;
			}
			if(genericObject->m_dataDefinitions[j].output)
			{
				QRectF dataArea(xo, y + outputIndex * dh, dataRectSize, dataRectSize);
				m_datas.append(qMakePair(dataArea, createdDatas[j].data()));
				++outputIndex;
			}
		}
		y += qMax(inputIndex, outputIndex) * dh;
		y += createdDataRectMargin;
	}

	// And the generic data
	QRectF dataArea(xi, y, dataRectSize, dataRectSize);
	m_datas.append(qMakePair(dataArea, (panda::BaseData*)genericObject->m_genericData));
}

void GenericObjectDrawStruct::drawDatas(QPainter* painter)
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

		if(dynamic_cast<panda::GenericData*>(dataPair.second))
			painter->drawRoundedRect(dataPair.first, 3, 3);
		else
			painter->drawRect(dataPair.first);
	}
}

QSize GenericObjectDrawStruct::getObjectSize()
{
	QSize objectSize(objectDefaultWidth, objectDefaultHeight);

	int nbInputs, nbOutputs;
	nbInputs = m_object->getInputDatas().size();
	nbOutputs = m_object->getOutputDatas().size();

	int nbCreated = genericObject->m_createdDatasStructs.size();
	int nbCreatedInputs = nbCreated * nbDefInputs;
	int nbCreatedOutputs = nbCreated * nbDefOutputs;

	nbInputs -= nbCreatedInputs + 1; // (Removing the generic data too)
	nbOutputs -= nbCreatedOutputs;

	int maxData = qMax(nbInputs, nbOutputs);
	int normalDatasH = qMax(0,(maxData-1)*dataRectMargin + maxData*dataRectSize);
	if(maxData)
		normalDatasH += createdDataRectMargin;
	normalDatasH += dataRectMargin + dataRectSize;	// Adding the generic data now

	int maxCreatedDatas = qMax(nbDefInputs, nbDefOutputs);
	int createdDatasH = (maxCreatedDatas-1)*dataRectMargin + maxCreatedDatas*dataRectSize;
	int totalCreatedDatasH = nbCreated*(createdDataRectMargin + dataRectMargin + createdDatasH);

	objectSize.rheight() = qMax(objectSize.rheight(), 2*dataStartY() + normalDatasH + totalCreatedDatasH);

	return objectSize;
}

int GenericObjectDrawClass = RegisterDrawObject<panda::GenericObject, GenericObjectDrawStruct>();
