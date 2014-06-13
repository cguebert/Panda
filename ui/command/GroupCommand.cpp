#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <ui/command/GroupCommand.h>

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
		if(m_document->isSelected(object.data()))
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
			m_document->selectionAdd(object.data());
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
			m_document->selectionAdd(object.data());
	}
}

void SelectObjectsInGroupCommand::undo()
{
	// If at least one of the object inside the group was selected, select the group
	for(auto object : m_group->getObjects())
	{
		if(m_document->isSelected(object.data()))
		{
			m_document->selectionAdd(m_group);
			return;
		}
	}
}

//****************************************************************************//

AddObjectToGroupCommand::AddObjectToGroupCommand(panda::Group* group,
											 QSharedPointer<panda::PandaObject> object,
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
	m_group->removeObject(m_object.data());
}

//****************************************************************************//

RemoveObjectFromGroupCommand::RemoveObjectFromGroupCommand(panda::Group* group,
											 QSharedPointer<panda::PandaObject> object,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_group(group)
	, m_object(object)
{
	setText(QCoreApplication::translate("RemoveObjectFromGroupCommand", "remove object from group"));
}

void RemoveObjectFromGroupCommand::redo()
{
	m_group->removeObject(m_object.data());
}

void RemoveObjectFromGroupCommand::undo()
{
	m_group->addObject(m_object);
}

//****************************************************************************//

EditGroupCommand::EditGroupCommand(panda::Group* group,
								   QString newName,
								   QVector<DataInfo> newDatas,
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
		info.data = data.data();
		info.name = data->getName();
		info.help = data->getHelp();
		m_prevDatas.push_back(info);
	}
	setText(QCoreApplication::translate("EditGroupCommand", "edit group"));
}

void EditGroupCommand::redo()
{
	m_group->m_groupName.setValue(m_newName);

	QMap< panda::BaseData*, QSharedPointer<panda::BaseData> > datasPtrMap;
	for(QSharedPointer<panda::BaseData> dataPtr : m_group->m_groupDatas)
		datasPtrMap.insert(dataPtr.data(), dataPtr);

	QList< QSharedPointer<panda::BaseData> > datasList;
	for(auto info : m_newDatas)
	{
		info.data->setName(info.name);
		info.data->setHelp(info.help);
		m_group->m_datas.removeAll(info.data);
		m_group->m_datas.push_back(info.data);
		datasList.push_back(datasPtrMap.value(info.data));
	}

	m_group->m_groupDatas = datasList;
	m_group->emitModified();
}

void EditGroupCommand::undo()
{
	m_group->m_groupName.setValue(m_prevName);

	QMap< panda::BaseData*, QSharedPointer<panda::BaseData> > datasPtrMap;
	for(QSharedPointer<panda::BaseData> dataPtr : m_group->m_groupDatas)
		datasPtrMap.insert(dataPtr.data(), dataPtr);

	QList< QSharedPointer<panda::BaseData> > datasList;
	for(auto info : m_prevDatas)
	{
		info.data->setName(info.name);
		info.data->setHelp(info.help);
		m_group->m_datas.removeAll(info.data);
		m_group->m_datas.push_back(info.data);
		datasList.push_back(datasPtrMap.value(info.data));
	}

	m_group->m_groupDatas = datasList;
	m_group->emitModified();
}
