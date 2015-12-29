#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <panda/command/GroupCommand.h>

#include <panda/helper/algorithm.h>

SelectGroupCommand::SelectGroupCommand(panda::PandaDocument* document,
									   panda::Group* group,
									   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_group(group)
{
	setText(QCoreApplication::translate("SelectGroupCommand", "select group"));
}

void SelectGroupCommand::redo()
{
	// If at least one of the object inside the group was selected, select the group
	for(auto object : m_group->getObjects())
	{
		if(m_document->isSelected(object.get()))
		{
			m_document->selectionAdd(m_group);
			return;
		}
	}
}

void SelectGroupCommand::undo()
{
	// If the group was selected, select all objects inside it
	if(m_document->isSelected(m_group))
	{
		for(auto object : m_group->getObjects())
			m_document->selectionAdd(object.get());
	}
}

//****************************************************************************//

SelectObjectsInGroupCommand::SelectObjectsInGroupCommand(panda::PandaDocument* document,
									   panda::Group* group,
									   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_group(group)
{
	setText(QCoreApplication::translate("SelectObjectsInGroupCommand", "select objects in group"));
}

void SelectObjectsInGroupCommand::redo()
{
	// If the group was selected, select all objects inside it
	if(m_document->isSelected(m_group))
	{
		for(auto object : m_group->getObjects())
			m_document->selectionAdd(object.get());
	}
}

void SelectObjectsInGroupCommand::undo()
{
	// If at least one of the object inside the group was selected, select the group
	for(auto object : m_group->getObjects())
	{
		if(m_document->isSelected(object.get()))
		{
			m_document->selectionAdd(m_group);
			return;
		}
	}
}

//****************************************************************************//

AddObjectToGroupCommand::AddObjectToGroupCommand(panda::Group* group,
											 std::shared_ptr<panda::PandaObject> object,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_group(group)
	, m_object(object)
{
	setText(QCoreApplication::translate("AddObjectToGroupCommand", "add object to group"));
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

RemoveObjectFromGroupCommand::RemoveObjectFromGroupCommand(panda::Group* group,
											 std::shared_ptr<panda::PandaObject> object,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_group(group)
	, m_object(object)
{
	setText(QCoreApplication::translate("RemoveObjectFromGroupCommand", "remove object from group"));
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

EditGroupCommand::EditGroupCommand(panda::Group* group,
								   std::string newName,
								   std::vector<DataInfo> newDatas,
								   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_group(group)
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
	setText(QCoreApplication::translate("EditGroupCommand", "edit group"));
}

void EditGroupCommand::redo()
{
	m_group->m_groupName.setValue(m_newName);

	std::map< panda::BaseData*, std::shared_ptr<panda::BaseData> > datasPtrMap;
	for(std::shared_ptr<panda::BaseData> dataPtr : m_group->m_groupDatas)
		datasPtrMap.emplace(dataPtr.get(), dataPtr);

	std::vector< std::shared_ptr<panda::BaseData> > datasList;
	for(auto info : m_newDatas)
	{
		info.data->setName(info.name);
		info.data->setHelp(info.help);
		panda::helper::removeAll(m_group->m_datas, info.data);
		m_group->m_datas.push_back(info.data);
		datasList.push_back(datasPtrMap.at(info.data));
	}

	m_group->m_groupDatas = datasList;
	m_group->emitModified();
}

void EditGroupCommand::undo()
{
	m_group->m_groupName.setValue(m_prevName);

	std::map< panda::BaseData*, std::shared_ptr<panda::BaseData> > datasPtrMap;
	for(std::shared_ptr<panda::BaseData> dataPtr : m_group->m_groupDatas)
		datasPtrMap.emplace(dataPtr.get(), dataPtr);

	std::vector< std::shared_ptr<panda::BaseData> > datasList;
	for(auto info : m_prevDatas)
	{
		info.data->setName(info.name);
		info.data->setHelp(info.help);
		panda::helper::removeAll(m_group->m_datas, info.data);
		m_group->m_datas.push_back(info.data);
		datasList.push_back(datasPtrMap.at(info.data));
	}

	m_group->m_groupDatas = datasList;
	m_group->emitModified();
}
