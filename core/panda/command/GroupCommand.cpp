#include <panda/command/GroupCommand.h>

#include <panda/document/PandaDocument.h>
#include <panda/object/Group.h>
#include <panda/document/ObjectsList.h>
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
	m_prevName = m_group->getGroupName();
	for(const auto& data : m_group->groupDatas().get())
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
	m_group->getGroupNameData().setValue(m_newName);

	std::map< BaseData*, std::shared_ptr<BaseData> > datasPtrMap;
	for(const auto& dataPtr : m_group->groupDatas().get())
		datasPtrMap.emplace(dataPtr.get(), dataPtr);

	{
		ModifiedSignalDisabler disabler { m_group };
		std::vector< std::shared_ptr<BaseData> > datasList;
		for (const auto& info : m_newDatas)
		{
			info.data->setName(info.name);
			info.data->setHelp(info.help);
			m_group->addData(info.data);
			datasList.push_back(datasPtrMap.at(info.data));
		}

		m_group->groupDatas().set(std::move(datasList));
	}

	m_group->emitModified();
}

void EditGroupCommand::undo()
{
	m_group->getGroupNameData().setValue(m_prevName);

	std::map< BaseData*, std::shared_ptr<BaseData> > datasPtrMap;
	for(const auto& dataPtr : m_group->groupDatas().get())
		datasPtrMap.emplace(dataPtr.get(), dataPtr);

	{
		ModifiedSignalDisabler disabler { m_group };
		std::vector< std::shared_ptr<BaseData> > datasList;
		for (const auto& info : m_prevDatas)
		{
			info.data->setName(info.name);
			info.data->setHelp(info.help);
			m_group->addData(info.data);
			datasList.push_back(datasPtrMap.at(info.data));
		}

		m_group->groupDatas().set(std::move(datasList));
	}

	m_group->emitModified();
}

} // namespace panda
