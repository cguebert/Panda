#include <panda/GenericObject.h>
#include <panda/PandaDocument.h>
#include <panda/DataFactory.h>

#include <QApplication>

#include <iostream>

namespace panda
{

using types::DataTypeId;

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

	registerFunctions();	// Create template functions

	m_genericData = data;
	m_genericData->setDisplayed(false);
	m_genericData->setPersistent(false);
	m_genericData->allowedTypes = getRegisteredTypes();
	m_dataDefinitions = defList;
}

BaseData* GenericObject::createDatas(int type)
{
	int valueType = types::DataTypeId::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = CreatedDatasStructPtr(new CreatedDatasStruct);
	createdDatasStruct->type = type;
	m_createdDatasStructs.append(createdDatasStruct);
	int index = m_createdDatasStructs.size();

	BaseData* firstInputData = nullptr;

	doEmitModified = false;
	doEmitDirty = false;

	int nbDefs = m_dataDefinitions.size();
	for(int i=0; i<nbDefs; ++i)
	{
		QString nameType = DataFactory::typeToName(type);
		QString dataName = m_dataDefinitions[i].name;
		if(dataName.contains("%1"))
			dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

		dataName += QString(" #%2").arg(index);	// Add the count

		int dataType = m_dataDefinitions[i].type;
		if(!DataTypeId::getValueType(dataType))	// Use the type of the connected Data
			dataType = types::DataTypeId::replaceValueType(dataType, valueType);

		BaseData* data = DataFactory::getInstance()->create(dataType, dataName, m_dataDefinitions[i].help, this);

		if(m_dataDefinitions[i].input)
		{
			addInput(data);
			if(!firstInputData)
				firstInputData = data;
		}

		if(m_dataDefinitions[i].output)
			addOutput(data);

		createdDatasStruct->datas.append(BaseDataPtr(data));
		m_createdDatasMap.insert(data, createdDatasStruct);
	}

	removeData(m_genericData);	// generic must always be last
	addData(m_genericData);

	doEmitModified = true;
	doEmitDirty = true;
	emitModified();

	return firstInputData;
}

void GenericObject::updateDataNames()
{
	int index = 1;
	int nbDefs = m_dataDefinitions.size();
	for(CreatedDatasStructPtr created : m_createdDatasStructs)
	{
		QString nameType = DataFactory::typeToName(created->type);
		for(int i=0; i<nbDefs; ++i)
		{
			QString dataName = m_dataDefinitions[i].name;
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
	int nbDefs = m_dataDefinitions.size();

	for(CreatedDatasStructPtr created : m_createdDatasStructs)
	{
		for(int i=0; i<nbDefs; ++i)
		{
			if(m_dataDefinitions[i].input)
				created->datas[i]->updateIfDirty();
		}

		DataList list;
		for(BaseDataPtr ptr : created->datas)
			list.append(ptr.data());
		invokeFunction(created->type, list);
	}

	cleanDirty();
}

void GenericObject::dataSetParent(BaseData* data, BaseData* parent)
{
	if(data == m_genericData)
	{
		int type = parent->getDataTrait()->valueTypeId();
		BaseData *inputData = createDatas(type);

		if(inputData)
			inputData->setParent(parent);

		emit modified(this);
	}
	else if(parent || !m_createdDatasMap.contains(data))
	{
		data->setParent(parent);
		emitModified();
	}
	else // (nullptr), we remove the data
	{
		data->setParent(nullptr);

		CreatedDatasStructPtr createdDatasStruct = m_createdDatasMap[data];
		int nbConnectedInputs = 0;
		for(BaseDataPtr d : createdDatasStruct->datas)
		{
			if(d->getParent())
				++nbConnectedInputs;
		}

		if(!nbConnectedInputs)	// We remove this group of datas
		{
			for(BaseDataPtr d : createdDatasStruct->datas)
			{
				removeData(d.data());
				m_createdDatasMap.remove(d.data());
			}

			m_createdDatasStructs.removeAll(createdDatasStruct);
			createdDatasStruct->datas.clear();
			updateDataNames();
		}

		emit modified(this);
	}
}

void GenericObject::save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected)
{
	for(CreatedDatasStructPtr created : m_createdDatasStructs)
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
	if(allowedTypes.size() && !allowedTypes.contains(parent->getDataTrait()->valueTypeId()))
		return false;
	return true;
}

QString GenericData::getDescription() const
{
	return QString("Accepting single values, lists & animations" + getTypesName());
}

QString GenericData::getTypesName() const
{
	if(allowedTypes.isEmpty())
		return "";
	QString temp("\n (");
	for(int i=0, nb=allowedTypes.size(); i<nb; ++i)
	{
		if(i)
			temp += ", ";
		if(i && !(i%3))
			temp += "\n  ";
		temp += DataFactory::typeToName(allowedTypes[i]);
	}

	temp += ")";
	return temp;
}

bool GenericSingleValueData::validParent(const BaseData* parent) const
{
	return parent->getDataTrait()->isSingleValue() && GenericData::validParent(parent);
}

QString GenericSingleValueData::getDescription() const
{
	return QString("Accepting single values" + getTypesName());
}

bool GenericVectorData::validParent(const BaseData* parent) const
{
	// TEST :  now accepting single values also, as the conversion is automatic
	return (parent->getDataTrait()->isVector()
			|| parent->getDataTrait()->isSingleValue())
			&& GenericData::validParent(parent);
}

QString GenericVectorData::getDescription() const
{
	return QString("Accepting lists" + getTypesName());
}

bool GenericAnimationData::validParent(const BaseData* parent) const
{
	return parent->getDataTrait()->isAnimation() && GenericData::validParent(parent);
}

QString GenericAnimationData::getDescription() const
{
	return QString("Accepting animations" + getTypesName());
}

} // namespace panda
