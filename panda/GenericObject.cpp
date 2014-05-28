#include <panda/GenericObject.h>
#include <panda/PandaDocument.h>
#include <panda/DataFactory.h>
#include <panda/types/DataTraits.h>
#include <panda/types/TypeConverter.h>

#include <ui/command/LinkDatasCommand.h>
#include <ui/command/RemoveGenericDataCommand.h>

#include <QApplication>

#include <iostream>

namespace panda
{

using types::DataTypeId;
using types::DataTraitsList;
using types::TypeConverter;

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

BaseData* GenericObject::createDatas(int type, int index)
{
	int valueType = types::DataTypeId::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = CreatedDatasStructPtr(new CreatedDatasStruct);
	createdDatasStruct->type = type;
	if(index >= 0 && index < m_createdDatasStructs.size())
		m_createdDatasStructs.insert(index, createdDatasStruct);
	else
		m_createdDatasStructs.append(createdDatasStruct);
	int nbCreated = m_createdDatasStructs.size();

	BaseData* firstInputData = nullptr;

	m_doEmitModified = false;
	m_doEmitDirty = false;

	int nbDefs = m_dataDefinitions.size();
	for(int i=0; i<nbDefs; ++i)
	{
		QString nameType = DataFactory::typeToName(type);
		QString dataName = m_dataDefinitions[i].name;
		if(dataName.contains("%1"))
			dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

		dataName += QString(" #%2").arg(nbCreated);	// Add the count

		int dataType = m_dataDefinitions[i].type;
		if(!dataType) // If the type in the definition is 0, use the full type of the connected Data
			dataType = type;
		else if(!DataTypeId::getValueType(dataType))	// Replace with the value type of the connected Data
			dataType = types::DataTypeId::replaceValueType(dataType, valueType);

		auto dataPtr = DataFactory::getInstance()->create(dataType, dataName, m_dataDefinitions[i].help, this);
		auto data = dataPtr.data();

		if(m_dataDefinitions[i].input)
		{
			addInput(data);
			if(!firstInputData)
				firstInputData = data;
		}

		if(m_dataDefinitions[i].output)
			addOutput(data);

		createdDatasStruct->datas.append(dataPtr);
		m_createdDatasMap.insert(data, createdDatasStruct);
	}

	if(index != -1)
	{
		reorderDatas();
		updateDataNames();
	}
	else
	{
		removeData(m_genericData);	// generic must always be last
		addData(m_genericData);
	}

	m_doEmitModified = true;
	m_doEmitDirty = true;
	emitModified();

	return firstInputData;
}

void GenericObject::reorderDatas()
{
	for(auto created : m_createdDatasStructs)
	{
		for(auto data : created->datas)
		{
			removeData(data.data());
			addData(data.data());
		}
	}

	removeData(m_genericData);	// generic must always be last
	addData(m_genericData);
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
			if(!created->datas[i])
				continue;

			QString dataName = m_dataDefinitions[i].name;
			if(dataName.contains("%1"))
				dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

			dataName += QString(" #%2").arg(index);	// Add the count
			created->datas[i]->setName(dataName);
		}

		++index;
	}
}

GenericData* const GenericObject::getGenericData()
{
	return m_genericData;
}

int GenericObject::nbOfCreatedDatas() const
{
	return m_createdDatasStructs.size();
}

bool GenericObject::isCreatedData(BaseData* data) const
{
	return m_createdDatasMap.contains(data);
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
		int type = m_genericData->getCompatibleType(parent);
		BaseData *inputData = createDatas(type);

		if(inputData)
			inputData->setParent(parent);

		m_parentDocument->onModifiedObject(this);
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
			// Create commmands so that we can undo the removal
			createUndoCommands(createdDatasStruct);

			for(BaseDataPtr d : createdDatasStruct->datas)
			{
				removeData(d.data());
				m_createdDatasMap.remove(d.data());
			}

			m_createdDatasStructs.removeAll(createdDatasStruct);
			createdDatasStruct->datas.clear();
			updateDataNames();
		}

		m_parentDocument->onModifiedObject(this);
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

void GenericObject::createUndoCommands(const CreatedDatasStructPtr& createdData)
{
	// Bugfix: don't do anything if we are currently deleting the object
	if(m_destructing)
		return;

	// Create commands if links are disconnected from the outputs of this group
	for(BaseDataPtr data : createdData->datas)
	{
		if(data->isOutput())
		{
			for(auto output : data->getOutputs())
			{
				BaseData* target = dynamic_cast<BaseData*>(output);
				if(target)
					m_parentDocument->addCommand(new LinkDatasCommand(target, nullptr));
			}
		}
	}

	// Create a command so that we can undo the removal of this group of datas
	auto currentCommand = m_parentDocument->getCurrentCommand();
	if(currentCommand)
	{
		int index = m_createdDatasStructs.indexOf(createdData);
		new RemoveGenericDataCommand(this, createdData->type, index, currentCommand);
	}
}

//***************************************************************//

SingleTypeGenericObject::SingleTypeGenericObject(PandaDocument *parent)
	: GenericObject(parent)
	, m_singleOutput(false)
	, m_connectedType(-1)
{
}

void SingleTypeGenericObject::update()
{
	int nbDefs = m_dataDefinitions.size();

	for(int i=0, nb=m_createdDatasStructs.size(); i<nb; ++i)
	{
		CreatedDatasStructPtr created = m_createdDatasStructs[i];

		DataList list;
		for(int j=0; j<nbDefs; ++j)
		{
			BaseDataPtr dataPtr = created->datas[j];
			if(m_dataDefinitions[j].input)
				dataPtr->updateIfDirty();

			if(m_singleOutput && i && m_dataDefinitions[j].output && !m_dataDefinitions[j].input)
				list.append(m_createdDatasStructs[0]->datas[j].data());
			else
				list.append(dataPtr.data());
		}

		invokeFunction(created->type, list);
	}

	cleanDirty();
}

BaseData* SingleTypeGenericObject::createDatas(int type, int index)
{
	if(m_connectedType == -1)
	{
		m_connectedType = type;
		m_genericData->allowedTypes.clear();
		m_genericData->allowedTypes.push_back(m_connectedType);
	}

	int valueType = types::DataTypeId::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = CreatedDatasStructPtr(new CreatedDatasStruct);
	createdDatasStruct->type = type;
	if(index >= 0 && index < m_createdDatasStructs.size())
		m_createdDatasStructs.insert(index, createdDatasStruct);
	else
		m_createdDatasStructs.append(createdDatasStruct);
	int nbCreated = m_createdDatasStructs.size();

	BaseData* firstInputData = nullptr;

	m_doEmitModified = false;
	m_doEmitDirty = false;

	int nbDefs = m_dataDefinitions.size();
	for(int i=0; i<nbDefs; ++i)
	{
		if(m_singleOutput && nbCreated > 1 && m_dataDefinitions[i].output && !m_dataDefinitions[i].input)
		{
			createdDatasStruct->datas.append(BaseDataPtr(nullptr));
		}
		else
		{
			QString nameType = DataFactory::typeToName(type);
			QString dataName = m_dataDefinitions[i].name;
			if(dataName.contains("%1"))
				dataName = dataName.arg(nameType);	// Insert the type's name into the data's name

			dataName += QString(" #%2").arg(nbCreated);	// Add the count

			int dataType = m_dataDefinitions[i].type;
			if(!dataType) // If the type in the definition is 0, use the full type of the connected Data
				dataType = type;
			else if(!DataTypeId::getValueType(dataType))	// Replace with the value type of the connected Data
				dataType = types::DataTypeId::replaceValueType(dataType, valueType);

			auto dataPtr = DataFactory::getInstance()->create(dataType, dataName, m_dataDefinitions[i].help, this);
			auto data = dataPtr.data();

			if(m_dataDefinitions[i].input)
			{
				addInput(data);
				if(!firstInputData)
					firstInputData = data;
			}

			if(m_dataDefinitions[i].output)
				addOutput(data);

			createdDatasStruct->datas.append(dataPtr);
			m_createdDatasMap.insert(data, createdDatasStruct);
		}
	}

	if(index != -1)
	{
		reorderDatas();
		updateDataNames();
	}
	else
	{
		removeData(m_genericData);	// generic must always be last
		addData(m_genericData);
	}

	m_doEmitModified = true;
	m_doEmitDirty = true;
	emitModified();

	return firstInputData;
}

void SingleTypeGenericObject::dataSetParent(BaseData* data, BaseData* parent)
{
	// New connection
	if(data == m_genericData)
	{
		int type = m_genericData->getCompatibleType(parent);
		BaseData *inputData = createDatas(type);

		if(inputData)
			inputData->setParent(parent);

		m_parentDocument->onModifiedObject(this);
	}
	// Changing connection
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
			if(d && d->getParent())
				++nbConnectedInputs;
		}

		if(!nbConnectedInputs)	// We remove this group of datas
		{
			// Create commmands so that we can undo the removal
			createUndoCommands(createdDatasStruct);

			// Last generic data
			bool lastGeneric = (m_createdDatasStructs.size() == 1);
			if(lastGeneric)
			{
				m_connectedType = -1;
				m_genericData->allowedTypes = getRegisteredTypes();
			}

			int nbDefs = m_dataDefinitions.size();
			for(int i=0; i<nbDefs; ++i)
			{
				BaseDataPtr dataPtr = createdDatasStruct->datas[i];
				if(m_singleOutput && m_dataDefinitions[i].output && !m_dataDefinitions[i].input)
				{
					if(lastGeneric)
					{
						removeData(dataPtr.data());
						m_createdDatasMap.remove(dataPtr.data());
					}
					else if(dataPtr) // Copy this data to the next created data
						m_createdDatasStructs[1]->datas[i] = dataPtr;
				}
				else
				{
					removeData(dataPtr.data());
					m_createdDatasMap.remove(dataPtr.data());
				}
			}

			m_createdDatasStructs.removeAll(createdDatasStruct);
			createdDatasStruct->datas.clear();
			updateDataNames();

			if(m_singleOutput)
				setDirtyValue(this);
		}

		m_parentDocument->onModifiedObject(this);
	}
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

QString GenericData::getTypesName(bool useFullDescription) const
{
	if(allowedTypes.isEmpty())
		return "";

	QVector<QString> sortedTypeNames;
	for(auto type : allowedTypes)
	{
		auto trait = DataTraitsList::getTrait(type);
		if(trait)
		{
			if(useFullDescription)
				sortedTypeNames.push_back(trait->typeDescription());
			else
				sortedTypeNames.push_back(trait->typeName());
		}
	}
	qSort(sortedTypeNames);

	QString types("\n (");
	for(int i=0, nb=sortedTypeNames.size(); i<nb; ++i)
	{
		if(i)
			types += ", ";
		if(i && !(i%3))
			types += "\n  ";
		types += sortedTypeNames[i];
	}

	types += ")";
	return types;
}

int GenericData::getCompatibleType(const BaseData* parent) const
{
	return parent->getDataTrait()->valueTypeId();
}

//***************************************************************//

bool GenericSingleValueData::validParent(const BaseData* parent) const
{
	return parent->getDataTrait()->isSingleValue() && GenericData::validParent(parent);
}

QString GenericSingleValueData::getDescription() const
{
	return QString("Accepting single values" + getTypesName());
}

//***************************************************************//

bool GenericVectorData::validParent(const BaseData* parent) const
{
	// Now accepting single values also, as the conversion is automatic
	return (parent->getDataTrait()->isVector()
			|| parent->getDataTrait()->isSingleValue())
			&& GenericData::validParent(parent);
}

QString GenericVectorData::getDescription() const
{
	return QString("Accepting lists" + getTypesName());
}

//***************************************************************//

bool GenericAnimationData::validParent(const BaseData* parent) const
{
	return parent->getDataTrait()->isAnimation() && GenericData::validParent(parent);
}

QString GenericAnimationData::getDescription() const
{
	return QString("Accepting animations" + getTypesName());
}

//***************************************************************//

bool GenericSpecificData::validParent(const BaseData* parent) const
{
	int fromType = parent->getDataTrait()->fullTypeId();
	if(allowedTypes.contains(fromType)) // Directly contains this type
		return true;

	return false;
}

QString GenericSpecificData::getDescription() const
{
	return QString("Accepting these types :" + getTypesName(true));
}

int GenericSpecificData::getCompatibleType(const BaseData* parent) const
{
	return parent->getDataTrait()->fullTypeId();
}

} // namespace panda
