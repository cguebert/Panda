#include <panda/object/GenericObject.h>
#include <panda/document/PandaDocument.h>
#include <panda/data/DataFactory.h>
#include <panda/types/DataTraits.h>
#include <panda/types/TypeConverter.h>
#include <panda/helper/algorithm.h>

#include <panda/command/LinkDatasCommand.h>
#include <panda/command/GenericDataCommand.h>

#include <iostream>

namespace panda
{

using types::DataTypeId;
using types::DataTraitsList;
using types::TypeConverter;

GenericObject::GenericObject(PandaDocument* parent)
	: PandaObject(parent)
{

}

GenericObject::~GenericObject()
{
	m_createdDatasMap.clear();
	m_createdDatasStructs.clear();
}

void GenericObject::setupGenericData(BaseGenericData& data, const GenericDataDefinitionList &defList)
{
	// Verify that there is no duplicate data name
	// And that there is at least one input data
	int nbInputDatas = 0, nbDefs = defList.size();
	for (int i = 0; i < nbDefs; ++i)
	{
		if (defList[i].isInput())
			nbInputDatas++;

		std::string name = defList[i].name;
		for (int j = i + 1; j < nbDefs; ++j)
		{
			if (name == defList[j].name)
			{
				std::cerr << "Fatal error : duplicate data name (" << name << ") in a GenericObject" << std::endl;
				std::terminate();
			}
		}
	}

	if (!nbInputDatas)
	{
		std::cerr << "Fatal error : no input data in a GenericObject" << std::endl;
		std::terminate();
	}

	m_genericData = &data;
	m_genericData->setDisplayed(false);
	m_genericData->setPersistent(false);
	m_genericData->m_allowedTypes = getRegisteredTypes();
	m_dataDefinitions = defList;
}

BaseData* GenericObject::createDatas(int type, int index)
{
	int valueType = types::DataTypeId::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = std::make_shared<CreatedDatasStruct>();
	createdDatasStruct->type = type;
	if(index >= 0 && index < static_cast<int>(m_createdDatasStructs.size()))
		m_createdDatasStructs.insert(m_createdDatasStructs.begin() + index, createdDatasStruct);
	else
		m_createdDatasStructs.push_back(createdDatasStruct);
	int nbCreated = m_createdDatasStructs.size();

	BaseData* firstInputData = nullptr;

	{
		ModifiedSignalDisabler modifiedDisabler { this };
		DirtySignalDisabler dirtyDisabler { this };

		int nbDefs = m_dataDefinitions.size();
		for (int i = 0; i < nbDefs; ++i)
		{
			std::string nameType = DataFactory::typeToName(type);
			std::string dataName = m_dataDefinitions[i].name;
			if (dataName.find("%1") != std::string::npos)
				helper::replaceAll(dataName, std::string("%1"), nameType);	// Insert the type's name into the data's name

			dataName += " #" + std::to_string(nbCreated);	// Add the count

			int dataType = m_dataDefinitions[i].type;
			if (!dataType) // If the type in the definition is 0, use the full type of the connected Data
				dataType = type;
			else if (!DataTypeId::getValueType(dataType))	// Replace with the value type of the connected Data
				dataType = types::DataTypeId::replaceValueType(dataType, valueType);

			auto dataPtr = DataFactory::create(dataType, dataName, m_dataDefinitions[i].help, this);
			auto data = dataPtr.get();

			if (m_dataDefinitions[i].isInput())
			{
				addInput(*data);
				if (!firstInputData)
					firstInputData = data;
			}

			if (m_dataDefinitions[i].isOutput())
				addOutput(*data);

			createdDatasStruct->datas.push_back(dataPtr);
			m_createdDatasMap.emplace(data, createdDatasStruct);
		}

		if (index != -1)
		{
			reorderDatas();
			updateDataNames();
		}
		else
		{
			removeData(m_genericData);	// generic must always be last
			addData(m_genericData);
		}

	}

	emitModified();

	return firstInputData;
}

void GenericObject::reorderDatas()
{
	for(auto created : m_createdDatasStructs)
	{
		for(auto data : created->datas)
		{
			removeData(data.get());
			addData(data.get());
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
		std::string nameType = DataFactory::typeToName(created->type);
		for(int i=0; i<nbDefs; ++i)
		{
			if(!created->datas[i])
				continue;

			std::string dataName = m_dataDefinitions[i].name;
			if(dataName.find("%1") != std::string::npos)
				helper::replaceAll(dataName, std::string("%1"), nameType);	// Insert the type's name into the data's name

			dataName += " #" + std::to_string(index);	// Add the index
			created->datas[i]->setName(dataName);
		}

		++index;
	}
}

void GenericObject::doUpdate(bool updateAllInputs)
{
	int nbDefs = m_dataDefinitions.size();

	for(CreatedDatasStructPtr created : m_createdDatasStructs)
	{
		if(updateAllInputs)
		{
			for(int i=0; i<nbDefs; ++i)
			{
				if(m_dataDefinitions[i].isInput())
					created->datas[i]->updateIfDirty();
			}
		}

		DataList list;
		list.reserve(created->datas.size());
		for(BaseDataPtr ptr : created->datas)
			list.push_back(ptr.get());
		invokeFunction(created->type, list);
	}
}

void GenericObject::dataSetParent(BaseData* data, BaseData* parent)
{
	if(data == m_genericData)
	{
		if (!parent) // When cancelling the connection to this object (see ConnectGenericDataCommand)
			return;

		if(!parentDocument()->getUndoStack().isExecuting()) // If a command is not currently creating the link
			createConnectCommand(parent);
	}
	else if (parent || !m_createdDatasMap.count(data))
	{
		PandaObject::dataSetParent(data, parent);
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
			createDisconnectCommands(createdDatasStruct);

			// Remove all datas in this group
			disconnectData(data);
		}

		parentDocument()->onModifiedObject(this);
	}
}

void GenericObject::disconnectData(BaseData* data)
{
	CreatedDatasStructPtr createdDatasStruct = m_createdDatasMap[data];
	for(BaseDataPtr d : createdDatasStruct->datas)
	{
		removeData(d.get());
		m_createdDatasMap.erase(d.get());
	}

	helper::removeAll(m_createdDatasStructs, createdDatasStruct);
	createdDatasStruct->datas.clear();
	updateDataNames();
}

std::vector<int> GenericObject::getRegisteredTypes()
{
	std::vector<int> keys;
	keys.reserve(m_functions.size());
	for (const auto& func : m_functions)
		keys.push_back(func.first);
	return keys;
}

void GenericObject::invokeFunction(int type, DataList& list)
{
	auto it = std::find_if(m_functions.begin(), m_functions.end(), [type](const TypeFuncPair& func){
		return func.first == type;
	});
	if (it != m_functions.end())
		it->second(list);
}

void GenericObject::save(XmlElement& elem, const std::vector<PandaObject*>* selected)
{
	for(CreatedDatasStructPtr created : m_createdDatasStructs)
	{
		auto e = elem.addChild("CreatedData");
		e.setAttribute("type", DataFactory::typeToName(created->type));
	}

	PandaObject::save(elem, selected);
}

void GenericObject::load(const XmlElement& elem)
{
	for(auto e = elem.firstChild("CreatedData"); e; e = e.nextSibling("CreatedData"))
		createDatas(DataFactory::nameToType(e.attribute("type").toString()));

	PandaObject::load(elem);
}

void GenericObject::createConnectCommand(BaseData* parent)
{
	auto& undoStack = parentDocument()->getUndoStack();
	undoStack.push(std::make_shared<ConnectGenericDataCommand>(this, parent));
}

void GenericObject::createDisconnectCommands(const CreatedDatasStructPtr& createdData)
{
	// Bugfix: don't do anything if we are currently deleting the object
	if(isDestructing())
		return;

	// Create commands if links are disconnected from the outputs of this group
	auto& undoStack = parentDocument()->getUndoStack();
	for(BaseDataPtr data : createdData->datas)
	{
		if(data && data->isOutput())
		{
			const auto outputs = data->getOutputs();
			for(auto output : outputs)
			{
				BaseData* target = dynamic_cast<BaseData*>(output);
				if(target)
					undoStack.push(std::make_shared<LinkDatasCommand>(target, nullptr));
			}
		}
	}

	// Create a command so that we can undo the removal of this group of datas
	auto currentCommand = undoStack.getCurrentCommand();
	if(currentCommand)
	{
		int index = helper::indexOf(m_createdDatasStructs, createdData);
		currentCommand->push(std::make_shared<RemoveGenericDataCommand>(this, createdData->type, index));
	}
}

//****************************************************************************//

SingleTypeGenericObject::SingleTypeGenericObject(PandaDocument* parent)
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
			if(m_dataDefinitions[j].isInput())
				dataPtr->updateIfDirty();

			if(m_singleOutput && i && m_dataDefinitions[j].isOutput() && !(m_dataDefinitions[j].isInput()))
				list.push_back(m_createdDatasStructs[0]->datas[j].get());
			else
				list.push_back(dataPtr.get());
		}

		invokeFunction(created->type, list);
	}
}

BaseData* SingleTypeGenericObject::createDatas(int type, int index)
{
	if(m_connectedType == -1)
	{
		m_connectedType = type;
		m_genericData->m_allowedTypes.clear();
		m_genericData->m_allowedTypes.push_back(m_connectedType);
	}

	int valueType = types::DataTypeId::getValueType(type);

	CreatedDatasStructPtr createdDatasStruct = CreatedDatasStructPtr(new CreatedDatasStruct);
	createdDatasStruct->type = type;
	if(index >= 0 && index < static_cast<int>(m_createdDatasStructs.size()))
		m_createdDatasStructs.insert(m_createdDatasStructs.begin() + index, createdDatasStruct);
	else
		m_createdDatasStructs.push_back(createdDatasStruct);
	int nbCreated = m_createdDatasStructs.size();

	BaseData* firstInputData = nullptr;

	{
		ModifiedSignalDisabler modifiedDisabler { this };
		DirtySignalDisabler dirtyDisabler { this };

		int nbDefs = m_dataDefinitions.size();
		for (int i = 0; i < nbDefs; ++i)
		{
			if (m_singleOutput && nbCreated > 1 && m_dataDefinitions[i].isOutput() && !m_dataDefinitions[i].isInput())
			{
				createdDatasStruct->datas.push_back(BaseDataPtr(nullptr));
			}
			else
			{
				std::string nameType = DataFactory::typeToName(type);
				std::string dataName = m_dataDefinitions[i].name;
				if (dataName.find("%1") != std::string::npos)
					helper::replaceAll(dataName, std::string("%1"), nameType);	// Insert the type's name into the data's name

				dataName += " #" + std::to_string(nbCreated);	// Add the count

				int dataType = m_dataDefinitions[i].type;
				if (!dataType) // If the type in the definition is 0, use the full type of the connected Data
					dataType = type;
				else if (!DataTypeId::getValueType(dataType))	// Replace with the value type of the connected Data
					dataType = types::DataTypeId::replaceValueType(dataType, valueType);

				auto dataPtr = DataFactory::create(dataType, dataName, m_dataDefinitions[i].help, this);
				auto data = dataPtr.get();

				if (m_dataDefinitions[i].isInput())
				{
					addInput(*data);
					if (!firstInputData)
						firstInputData = data;
				}

				if (m_dataDefinitions[i].isOutput())
					addOutput(*data);

				createdDatasStruct->datas.push_back(dataPtr);
				m_createdDatasMap.emplace(data, createdDatasStruct);
			}
		}

		if (index != -1)
		{
			reorderDatas();
			updateDataNames();
		}
		else
		{
			removeData(m_genericData);	// generic must always be last
			addData(m_genericData);
		}

	}

	emitModified();

	return firstInputData;
}

void SingleTypeGenericObject::dataSetParent(BaseData* data, BaseData* parent)
{
	// New connection
	if(data == m_genericData)
	{
		if (!parent) // When cancelling the connection to this object (see ConnectGenericDataCommand)
			return;

		if(!parentDocument()->getUndoStack().isExecuting()) // If a command is not currently creating the link
			createConnectCommand(parent);
	}
	// Changing connection
	else if(parent || !m_createdDatasMap.count(data))
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
			createDisconnectCommands(createdDatasStruct);

			disconnectData(data);
		}

		parentDocument()->onModifiedObject(this);
	}
}

void SingleTypeGenericObject::disconnectData(BaseData* data)
{
	CreatedDatasStructPtr createdDatasStruct = m_createdDatasMap[data];

	// Last generic data
	bool lastGeneric = (m_createdDatasStructs.size() == 1);
	if(lastGeneric)
	{
		m_connectedType = -1;
		m_genericData->m_allowedTypes = getRegisteredTypes();
	}

	int nbDefs = m_dataDefinitions.size();
	for(int i=0; i<nbDefs; ++i)
	{
		BaseDataPtr dataPtr = createdDatasStruct->datas[i];
		if(m_singleOutput && m_dataDefinitions[i].isOutput() && !m_dataDefinitions[i].isInput())
		{
			if(lastGeneric)
			{
				removeData(dataPtr.get());
				m_createdDatasMap.erase(dataPtr.get());
			}
			else if(dataPtr) // Copy this data to the next created data
				m_createdDatasStructs[1]->datas[i] = dataPtr;
		}
		else
		{
			removeData(dataPtr.get());
			m_createdDatasMap.erase(dataPtr.get());
		}
	}

	helper::removeAll(m_createdDatasStructs, createdDatasStruct);
	createdDatasStruct->datas.clear();
	updateDataNames();

	if(m_singleOutput)
		setDirtyValue(this);
}

//****************************************************************************//

bool BaseGenericData::validParent(const BaseData* parent) const
{
	if(m_allowedTypes.size() && !helper::contains(m_allowedTypes, parent->getDataTrait()->valueTypeId()))
		return false;
	return true;
}

std::string BaseGenericData::getTypesName(bool useFullDescription) const
{
	if(m_allowedTypes.empty())
		return "";

	std::vector<std::string> sortedTypeNames;
	for(auto type : m_allowedTypes)
	{
		auto trait = DataTraitsList::getTrait(type);
		if(trait)
		{
			if(useFullDescription)
				sortedTypeNames.push_back(trait->typeDescription());
			else
				sortedTypeNames.push_back(trait->valueTypeName());
		}
	}
	std::sort(sortedTypeNames.begin(), sortedTypeNames.end());

	std::string types("\n (");
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

int BaseGenericData::getCompatibleType(const BaseData* parent) const
{
	return parent->getDataTrait()->valueTypeId();
}

//****************************************************************************//

std::string GenericData::getDescription() const
{
	return std::string("Accepting single values, lists & animations" + getTypesName());
}

//****************************************************************************//

bool GenericSingleValueData::validParent(const BaseData* parent) const
{
	return parent->getDataTrait()->isSingleValue() && BaseGenericData::validParent(parent);
}

std::string GenericSingleValueData::getDescription() const
{
	return std::string("Accepting single values" + getTypesName());
}

//****************************************************************************//

bool GenericVectorData::validParent(const BaseData* parent) const
{
	// Now accepting single values also, as the conversion is automatic
	return (parent->getDataTrait()->isVector()
			|| parent->getDataTrait()->isSingleValue())
			&& BaseGenericData::validParent(parent);
}

std::string GenericVectorData::getDescription() const
{
	return std::string("Accepting lists" + getTypesName());
}

//****************************************************************************//

bool GenericAnimationData::validParent(const BaseData* parent) const
{
	return parent->getDataTrait()->isAnimation() && BaseGenericData::validParent(parent);
}

std::string GenericAnimationData::getDescription() const
{
	return std::string("Accepting animations" + getTypesName());
}

//****************************************************************************//

bool GenericSpecificData::validParent(const BaseData* parent) const
{
	int fromType = parent->getDataTrait()->fullTypeId();
	if(helper::contains(m_allowedTypes, fromType)) // Directly contains this type
		return true;

	return false;
}

std::string GenericSpecificData::getDescription() const
{
	return std::string("Accepting these types :" + getTypesName(true));
}

int GenericSpecificData::getCompatibleType(const BaseData* parent) const
{
	return parent->getDataTrait()->fullTypeId();
}

} // namespace panda
