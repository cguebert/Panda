#include <panda/GenericObject.h>
#include <panda/PandaDocument.h>

#include <QApplication>

#include <iostream>

namespace panda
{

GenericObject::GenericObject(PandaDocument *parent)
	: PandaObject(parent)
	, setParentProtection(false)
{

}

GenericObject::~GenericObject()
{
	foreach(CreatedDatasStructPtr createdDatasStruct, _createdDatasStructs)
	{
		foreach(BaseData* d, createdDatasStruct->datas)
		{
			removeData(d);
			_createdDatasMap.remove(d);
			delete d;
		}

		createdDatasStruct->datas.clear();
		_createdDatasStructs.removeAll(createdDatasStruct);
		delete createdDatasStruct;
	}
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

	_genericData = data;
	_genericData->setDisplayed(false);
	_genericData->setPersistent(false);
	_genericData->allowedTypes = getRegisteredTypes();
	_dataDefinitions = defList;

	this->registerFunctions();	// Create template functions
}

BaseData* GenericObject::createDatas(int type)
{
	int valueType = BaseData::getValueType(type);

	CreatedDatasStruct* createdDatasStruct = new CreatedDatasStruct;
	createdDatasStruct->type = type;
	_createdDatasStructs.append(CreatedDatasStructPtr(createdDatasStruct));
	int index = _createdDatasStructs.size();

	BaseData* firstInputData = NULL;

	int nbDefs = _dataDefinitions.size();
	for(int i=0; i<nbDefs; ++i)
	{
		QString nameType = BaseData::typeToName(type);
		QString dataName = _dataDefinitions[i].name;
		if(dataName.contains("%1"))
			dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

		dataName += QString(" #%2").arg(index);	// Add the count

		int dataType = _dataDefinitions[i].type;
		if(!BaseData::getValueType(dataType))	// Use the type of the connected Data
			dataType = BaseData::replaceValueType(dataType, valueType);

		BaseData* data = createDataFromFullType(dataType, dataName, _dataDefinitions[i].help, this);

		if(_dataDefinitions[i].input)
		{
			addInput(data);
			if(!firstInputData)
				firstInputData = data;
		}

		if(_dataDefinitions[i].output)
			addOutput(data);

		createdDatasStruct->datas.append(data);
		_createdDatasMap[data] = createdDatasStruct;
	}

	removeData(_genericData);	// generic must always be last
	addData(_genericData);

	return firstInputData;
}

void GenericObject::updateDataNames()
{
	int index = 1;
	int nbDefs = _dataDefinitions.size();
	foreach(CreatedDatasStructPtr created, _createdDatasStructs)
	{
		QString nameType = BaseData::typeToName(created->type);
		for(int i=0; i<nbDefs; ++i)
		{
			QString dataName = _dataDefinitions[i].name;
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
	foreach(CreatedDatasStructPtr created, _createdDatasStructs)
		this->invokeFunction(created->type, created->datas);

	this->cleanDirty();
}

void GenericObject::dataSetParent(BaseData* data, BaseData* parent)
{
	if(setParentProtection)
		return;
	setParentProtection = true;

	if(data == _genericData)
	{
		int type = parent->getValueType();
		BaseData *inputData = createDatas(type);

		if(inputData)
			inputData->setParent(parent);

		emit modified(this);
	}
	else if(parent || !_createdDatasMap.contains(data))
	{
		data->setParent(parent);
	}
	else // (NULL), we remove the data
	{
		data->setParent(NULL);

		CreatedDatasStruct* createdDatasStruct = _createdDatasMap[data];
		int nbConnectedInputs = 0;
		foreach(BaseData* d, createdDatasStruct->datas)
		{
			if(d->getParent())
				++nbConnectedInputs;
		}

		if(!nbConnectedInputs)	// We remove this group of datas
		{
			foreach(BaseData* d, createdDatasStruct->datas)
			{
				removeData(d);
				_createdDatasMap.remove(d);
				delete d;
			}

			createdDatasStruct->datas.clear();
			_createdDatasStructs.removeAll(createdDatasStruct);
			delete createdDatasStruct;
//			_createdDatasStructs.removeAll(CreatedDatasStructPtr(createdDatasStruct));
			updateDataNames();
		}

		emit modified(this);
	}

	setParentProtection = false;
}

void GenericObject::save(QDataStream& out)
{
	out << (quint32)_createdDatasStructs.size();
	foreach(CreatedDatasStructPtr created, _createdDatasStructs)
		out << (quint32)created->type;

	PandaObject::save(out);
}

void GenericObject::save(QTextStream& out)
{
	out << (quint32)_createdDatasStructs.size() << " ";
	foreach(CreatedDatasStructPtr created, _createdDatasStructs)
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
