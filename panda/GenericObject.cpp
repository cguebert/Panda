#include <panda/GenericObject.h>
#include <panda/PandaDocument.h>

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
/*	foreach(CreatedDatasStructPtr createdDatasStruct, createdDatasStructs_)
	{
		foreach(BaseData* d, createdDatasStruct->datas)
		{
			removeData(d);
			createdDatasMap_.remove(d);
			delete d;
		}

		createdDatasStruct->datas.clear();
		createdDatasStructs_.removeAll(createdDatasStruct);
		delete createdDatasStruct;
	}	*/
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

	genericData_ = data;
	genericData_->setDisplayed(false);
	genericData_->setPersistent(false);
	genericData_->allowedTypes = getRegisteredTypes();
	dataDefinitions_ = defList;

	this->registerFunctions();	// Create template functions
}

BaseData* GenericObject::createDatas(int type)
{
	int valueType = BaseData::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = CreatedDatasStructPtr(new CreatedDatasStruct);
	createdDatasStruct->type = type;
	createdDatasStructs_.append(createdDatasStruct);
	int index = createdDatasStructs_.size();

	BaseData* firstInputData = NULL;

	this->doEmitModified = false;

	int nbDefs = dataDefinitions_.size();
	for(int i=0; i<nbDefs; ++i)
	{
		QString nameType = BaseData::typeToName(type);
		QString dataName = dataDefinitions_[i].name;
		if(dataName.contains("%1"))
			dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

		dataName += QString(" #%2").arg(index);	// Add the count

		int dataType = dataDefinitions_[i].type;
		if(!BaseData::getValueType(dataType))	// Use the type of the connected Data
			dataType = BaseData::replaceValueType(dataType, valueType);

		BaseData* data = createDataFromFullType(dataType, dataName, dataDefinitions_[i].help, this);

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

	this->doEmitModified = true;
	emitModified();

	return firstInputData;
}

void GenericObject::updateDataNames()
{
	int index = 1;
	int nbDefs = dataDefinitions_.size();
	foreach(CreatedDatasStructPtr created, createdDatasStructs_)
	{
		QString nameType = BaseData::typeToName(created->type);
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
	else // (NULL), we remove the data
	{
		data->setParent(NULL);

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

void GenericObject::save(QDataStream& out)
{
	out << (quint32)createdDatasStructs_.size();
	foreach(CreatedDatasStructPtr created, createdDatasStructs_)
		out << (quint32)created->type;

	PandaObject::save(out);
}

void GenericObject::save(QTextStream& out)
{
	out << (quint32)createdDatasStructs_.size() << " ";
	foreach(CreatedDatasStructPtr created, createdDatasStructs_)
		out << (quint32)created->type << " ";

	PandaObject::save(out);
}

void GenericObject::load(QDataStream& in)
{
	quint32 nb;
	in >> nb;

	for(quint32 i=0; i<nb; ++i)
	{
		quint32 type;
		in >> type;
		createDatas(type);
	}

	PandaObject::load(in);
}

void GenericObject::load(QTextStream& in)
{
	quint32 nb;
	in >> nb;

	for(quint32 i=0; i<nb; ++i)
	{
		quint32 type;
		in >> type;
		createDatas(type);
	}

	in.skipWhiteSpace();
	PandaObject::load(in);
}

} // namespace panda
