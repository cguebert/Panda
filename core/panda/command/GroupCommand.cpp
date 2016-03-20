#include <panda/PandaDocument.h>
#include <panda/object/Group.h>

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
	m_group->addObject(m_object);
}

void AddObjectToGroupCommand::undo()
{
	m_group->removeObject(m_object.get());
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
	m_group->removeObject(m_object.get());
}

void RemoveObjectFromGroupCommand::undo()
{
	m_group->addObject(m_object);
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
	for(std::shared_ptr<BaseData> dataPtr : m_group->m_groupDatas)
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
	for(std::shared_ptr<BaseData> dataPtr : m_group->m_groupDatas)
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

} // namespace panda
