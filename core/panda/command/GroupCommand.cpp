#include <panda/PandaDocument.h>
#include <panda/object/Group.h>
#include <panda/document/ObjectsList.h>

#include <panda/command/GroupCommand.h>

#include <panda/helper/algorithm.h>

namespace panda
{

AddObjectToGroupCommand::AddObjectToGroupCommand(Group* group,
											 std::shared_ptr<PandaObject> object)
	: m_group(group)
	, m_object(object)
{
	setText("add object to group");
}

void AddObjectToGroupCommand::redo()
{
	m_group->getObjectsList().addObject(m_object);
}

void AddObjectToGroupCommand::undo()
{
	m_group->getObjectsList().removeObject(m_object.get());
}

//****************************************************************************//

RemoveObjectFromGroupCommand::RemoveObjectFromGroupCommand(Group* group,
											 std::shared_ptr<PandaObject> object)
	: m_group(group)
	, m_object(object)
{
	setText("remove object from group");
}

void RemoveObjectFromGroupCommand::redo()
{
	m_group->getObjectsList().removeObject(m_object.get());
}

void RemoveObjectFromGroupCommand::undo()
{
	m_group->getObjectsList().addObject(m_object);
}

//****************************************************************************//

EditGroupCommand::EditGroupCommand(Group* group,
								   std::string newName,
								   std::vector<DataInfo> newDatas)
	: m_group(group)
	, m_newName(newName)
	, m_newDatas(newDatas)
{
	m_prevName = group->getGroupName();
	for(auto data : m_group->m_groupDatas)
	{
		DataInfo info;
		info.data = data.get();
		info.name = data->getName();
		info.help = data->getHelp();
		m_prevDatas.push_back(info);
	}
	setText("edit group");
}

void EditGroupCommand::redo()
{
	m_group->m_groupName.setValue(m_newName);

	std::map< BaseData*, std::shared_ptr<BaseData> > datasPtrMap;
	for(const auto& dataPtr : m_group->m_groupDatas)
		datasPtrMap.emplace(dataPtr.get(), dataPtr);

	m_group->enableModifiedSignal(false);
	std::vector< std::shared_ptr<BaseData> > datasList;
	for(auto info : m_newDatas)
	{
		info.data->setName(info.name);
		info.data->setHelp(info.help);
		m_group->addData(info.data);
		datasList.push_back(datasPtrMap.at(info.data));
	}

	m_group->m_groupDatas = datasList;
	m_group->enableModifiedSignal(true);
	m_group->emitModified();
}

void EditGroupCommand::undo()
{
	m_group->m_groupName.setValue(m_prevName);

	std::map< BaseData*, std::shared_ptr<BaseData> > datasPtrMap;
	for(const auto& dataPtr : m_group->m_groupDatas)
		datasPtrMap.emplace(dataPtr.get(), dataPtr);

	m_group->enableModifiedSignal(false);
	std::vector< std::shared_ptr<BaseData> > datasList;
	for(auto info : m_prevDatas)
	{
		info.data->setName(info.name);
		info.data->setHelp(info.help);
		m_group->addData(info.data);
		datasList.push_back(datasPtrMap.at(info.data));
	}

	m_group->m_groupDatas = datasList;
	m_group->enableModifiedSignal(true);
	m_group->emitModified();
}

//****************************************************************************//

AddDataToGroupCommand::AddDataToGroupCommand(Group* group, std::shared_ptr<BaseData> data)
	: m_group(group)
	, m_data(data)
{
	setText("add group data");
}

void AddDataToGroupCommand::redo()
{
	m_group->addGroupData(m_data);
	m_group->emitModified();
}

void AddDataToGroupCommand::undo()
{
	m_group->removeGroupData(m_data);
	m_group->emitModified();
}

//****************************************************************************//

RemoveDataFromGroupCommand::RemoveDataFromGroupCommand(Group* group, BaseData* data)
	: m_group(group)
{
	setText("remove group data");

	const auto& groupDatas = m_group->getGroupDatas();
	auto gIt = std::find_if(groupDatas.begin(), groupDatas.end(), [data](const std::shared_ptr<BaseData>& dataSPtr) {
		return dataSPtr.get() == data;
	});
	m_data = *gIt;
	m_groupDataIndex = std::distance(groupDatas.begin(), gIt);

	const auto& datas = m_group->getDatas();
	auto dIt = std::find_if(datas.begin(), datas.end(), [data](const BaseData* dataPtr) {
		return dataPtr == data;
	});
	m_dataIndex = std::distance(datas.begin(), dIt);

	m_input = data->isInput();
	m_output = data->isOutput();
}

void RemoveDataFromGroupCommand::redo()
{
	if (m_input)
		m_group->removeInput(*m_data);
	if (m_output)
		m_group->removeOutput(*m_data);

	m_group->removeGroupData(m_data);
	m_group->removeData(m_data.get());
}

void RemoveDataFromGroupCommand::undo()
{
	if (m_input)
		m_group->addInput(*m_data);
	if (m_output)
	{
		m_data->setOutput(true);
		m_group->addOutput(*m_data);
	}

	m_group->addGroupData(m_data, m_groupDataIndex);
	m_group->addData(m_data.get(), m_dataIndex);
}

//****************************************************************************//

ReorderGroupDataCommand::ReorderGroupDataCommand(Group* group, BaseData* data, int index)
	: m_group(group)
{
	setText("reorder group data");
}

void ReorderGroupDataCommand::redo()
{
}

void ReorderGroupDataCommand::undo()
{
}
} // namespace panda
