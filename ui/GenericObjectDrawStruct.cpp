#include <panda/PandaDocument.h>

#include <ui/GraphView.h>
#include <ui/GenericObjectDrawStruct.h>

#include <QPainter>

using panda::GenericObject;

GenericObjectDrawStruct::GenericObjectDrawStruct(GraphView* view, panda::GenericObject* object)
	: ObjectDrawStruct(view, (panda::PandaObject*)object)
	, genericObject(object)
	, nbDefInputs(0)
	, nbDefOutputs(0)
{
	update();

	foreach(GenericObject::GenericDataDefinition def, object->dataDefinitions_)
	{
		if(def.input)
			++nbDefInputs;
		if(def.output)
			++nbDefOutputs;
	}
}

void GenericObjectDrawStruct::update()
{
	objectArea = QRectF(position + parentView->getViewDelta(), this->getObjectSize());

	datas.clear();
	QList<panda::BaseData*> inputDatas, outputDatas;
	inputDatas = object->getInputDatas();
	outputDatas = object->getOutputDatas();
	int nbInputs = inputDatas.size(), nbOutputs = outputDatas.size();

	int xi = objectArea.x() + dataRectMargin;
	int xo = objectArea.right() - dataRectMargin - dataRectSize;
	int startY = objectArea.y() + dataStartY();
	int dh = dataRectSize + dataRectMargin;
	int y;

	// First the "normal" datas
	int index = 0;
	for(int i=0; i<nbInputs; ++i)
	{
		panda::BaseData* data = inputDatas[i];
		if(!genericObject->createdDatasMap_.contains(data) && data != genericObject->genericData_)
		{
			QRectF dataArea(xi, startY + index * dh,
							dataRectSize, dataRectSize);
			datas.append(qMakePair(dataArea, data));
			++index;
		}
	}
	y = startY + index * dh;

	index = 0;
	for(int i=0; i<nbOutputs; ++i)
	{
		panda::BaseData* data = outputDatas[i];
		if(!genericObject->createdDatasMap_.contains(data))
		{
			QRectF dataArea(xo, startY + index * dh,
							dataRectSize, dataRectSize);
			datas.append(qMakePair(dataArea, data));
			++index;
		}
	}
	y = qMax(y, startY + index * dh);

	// Now the created datas
	y += createdDataRectMargin;
	int nbCreated = genericObject->createdDatasStructs_.size();
	int nbDef = genericObject->dataDefinitions_.size();
	for(int i=0; i<nbCreated; ++i)
	{
		GenericObject::DataPtrList createdDatas = genericObject->createdDatasStructs_[i]->datas;
		int inputIndex = 0, outputIndex = 0;
		for(int j=0; j<nbDef; ++j)
		{
			if(genericObject->dataDefinitions_[j].input)
			{
				QRectF dataArea(xi, y + inputIndex * dh, dataRectSize, dataRectSize);
				datas.append(qMakePair(dataArea, createdDatas[j].data()));
				++inputIndex;
			}
			if(genericObject->dataDefinitions_[j].output)
			{
				QRectF dataArea(xo, y + outputIndex * dh, dataRectSize, dataRectSize);
				datas.append(qMakePair(dataArea, createdDatas[j].data()));
				++outputIndex;
			}
		}
		y += qMax(inputIndex, outputIndex) * dh;
		y += createdDataRectMargin;
	}

	// And the generic data
	QRectF dataArea(xi, y, dataRectSize, dataRectSize);
	datas.append(qMakePair(dataArea, (panda::BaseData*)genericObject->genericData_));
}

void GenericObjectDrawStruct::drawDatas(QPainter* painter)
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
	nbInputs = object->getInputDatas().size();
	nbOutputs = object->getOutputDatas().size();

	int nbCreated = genericObject->createdDatasStructs_.size();
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
