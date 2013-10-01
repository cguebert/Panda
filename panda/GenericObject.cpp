#include <panda/GenericObject.h>
#include <panda/PandaDocument.h>
#include <panda/DataFactory.h>

#include <QApplication>

#include <iostream>

namespace panda
{

GenericObject::GenericObject(PandaDocument *parent)
	: PandaObject(parent)
{

}

GenericObject::~GenericObject()
{
}

void GenericObject::setupGenericObject(GenericData* data, const GenericDataDefinitionList &defList)
{
	// Verify that there is no duplicate data name
	// And that there is at least one input data
	int nbInputDatas = 0;
	for(int i=0; i<defList.size(); ++i)
	{
		if(defList[i].input)
			nbInputDatas++;

		QString name = defList[i].name;
		for(int j=i+1; j<defList.size(); ++j)
		{
			if(name == defList[j].name)
			{
				std::cerr << "Fatal error : duplicate data name (" << name.toStdString() << ") in a GenericObject" << std::endl;
				QCoreApplication::exit(-2);
			}
		}
	}

	if(!nbInputDatas)
	{
		std::cerr << "Fatal error : no input data in a GenericObject" << std::endl;
		QCoreApplication::exit(-3);
	}

	this->registerFunctions();	// Create template functions

	genericData_ = data;
	genericData_->setDisplayed(false);
	genericData_->setPersistent(false);
	genericData_->allowedTypes = getRegisteredTypes();
	dataDefinitions_ = defList;
}

BaseData* GenericObject::createDatas(int type)
{
	int valueType = BaseData::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = CreatedDatasStructPtr(new CreatedDatasStruct);
	createdDatasStruct->type = type;
	createdDatasStructs_.append(createdDatasStruct);
	int index = createdDatasStructs_.size();

	BaseData* firstInputData = nullptr;

	doEmitModified = false;
	doEmitDirty = false;

	int nbDefs = dataDefinitions_.size();
	for(int i=0; i<nbDefs; ++i)
	{
		QString nameType = DataFactory::typeToName(type);
		QString dataName = dataDefinitions_[i].name;
		if(dataName.contains("%1"))
			dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

		dataName += QString(" #%2").arg(index);	// Add the count

		int dataType = dataDefinitions_[i].type;
		if(!BaseData::getValueType(dataType))	// Use the type of the connected Data
			dataType = BaseData::replaceValueType(dataType, valueType);

		BaseData* data = DataFactory::getInstance()->create(dataType, dataName, dataDefinitions_[i].help, this);

		if(dataDefinitions_[i].input)
		{
			addInput(data);
			if(!firstInputData)
				firstInputData = data;
		}

		if(dataDefinitions_[i].output)
			addOutput(data);

		createdDatasStruct->datas.append(BaseDataPtr(data));
		createdDatasMap_.insert(data, createdDatasStruct);
	}

	removeData(genericData_);	// generic must always be last
	addData(genericData_);

	doEmitModified = true;
	doEmitDirty = true;
	emitModified();

	return firstInputData;
}

void GenericObject::updateDataNames()
{
	int index = 1;
	int nbDefs = dataDefinitions_.size();
	foreach(CreatedDatasStructPtr created, createdDatasStructs_)
	{
		QString nameType = DataFactory::typeToName(created->type);
		for(int i=0; i<nbDefs; ++i)
		{
			QString dataName = dataDefinitions_[i].name;
			if(dataName.contains("%1"))
				dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

			dataName += QString(" #%2").arg(index);	// Add the count
			created->datas[i]->setName(dataName);
		}

		++index;
	}
}

void GenericObject::update()
{
	int nbDefs = dataDefinitions_.size();

	foreach(CreatedDatasStructPtr created, createdDatasStructs_)
	{
		for(int i=0; i<nbDefs; ++i)
		{
			if(dataDefinitions_[i].input)
				created->datas[i]->updateIfDirty();
		}

		DataList list;
		foreach(BaseDataPtr ptr, created->datas)
			list.append(ptr.data());
		this->invokeFunction(created->type, list);
	}

	this->cleanDirty();
}

void GenericObject::dataSetParent(BaseData* data, BaseData* parent)
{
	if(data == genericData_)
	{
		int type = parent->getValueType();
		BaseData *inputData = createDatas(type);

		if(inputData)
			inputData->setParent(parent);

		emit modified(this);
	}
	else if(parent || !createdDatasMap_.contains(data))
	{
		data->setParent(parent);
	}
	else // (nullptr), we remove the data
	{
		data->setParent(nullptr);

		CreatedDatasStructPtr createdDatasStruct = createdDatasMap_[data];
		int nbConnectedInputs = 0;
		foreach(BaseDataPtr d, createdDatasStruct->datas)
		{
			if(d->getParent())
				++nbConnectedInputs;
		}

		if(!nbConnectedInputs)	// We remove this group of datas
		{
			foreach(BaseDataPtr d, createdDatasStruct->datas)
			{
				removeData(d.data());
				createdDatasMap_.remove(d.data());
			}

			createdDatasStruct->datas.clear();
			createdDatasStructs_.removeAll(createdDatasStruct);
			updateDataNames();
		}

		emit modified(this);
	}
}

void GenericObject::save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected)
{
	foreach(CreatedDatasStructPtr created, createdDatasStructs_)
	{
		QDomElement e = doc.createElement("CreatedData");
		e.setAttribute("type", DataFactory::typeToName(created->type));
		elem.appendChild(e);
	}

	PandaObject::save(doc, elem, selected);
}

void GenericObject::load(QDomElement& elem)
{
	QDomElement e = elem.firstChildElement("CreatedData");
	while(!e.isNull())
	{
		createDatas(DataFactory::nameToType(e.attribute("type")));
		e = e.nextSiblingElement("CreatedData");
	}

	PandaObject::load(elem);
}

//***************************************************************//

bool GenericData::validParent(const BaseData* parent) const
{
	if(allowedTypes.size() && !allowedTypes.contains(parent->getValueType()))
		return false;
	return true;
}

QString GenericData::getDescription() const
{
	return QString("Accepting all types");
}

bool GenericSingleValueData::validParent(const BaseData* parent) const
{
	return parent->isSingleValue() && GenericData::validParent(parent);
}

QString GenericSingleValueData::getDescription() const
{
	return QString("Accepting single values");
}

bool GenericVectorData::validParent(const BaseData* parent) const
{
	// TEST :  now accepting single values also, as the conversion is automatic
	return (parent->isVector() || parent->isSingleValue()) && GenericData::validParent(parent);
}

QString GenericVectorData::getDescription() const
{
	return QString("Accepting lists");
}

bool GenericAnimationData::validParent(const BaseData* parent) const
{
	return parent->isAnimation() && GenericData::validParent(parent);
}

QString GenericAnimationData::getDescription() const
{
	return QString("Accepting animations");
}

} // namespace panda
