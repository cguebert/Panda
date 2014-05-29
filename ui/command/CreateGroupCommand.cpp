#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <ui/command/CreateGroupCommand.h>

CreateGroupCommand::CreateGroupCommand(panda::PandaDocument* document,
									   panda::Group* group,
									   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_group(group)
{
	setText(QCoreApplication::translate("CreateGroupCommand", "detach dockable object"));
}

void CreateGroupCommand::redo()
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

void CreateGroupCommand::undo()
{
	// If the group was selected, select all objects inside it
	if(m_document->isSelected(m_group))
	{
		for(auto object : m_group->getObjects())
			m_document->selectionAdd(object.data());
	}
}

//***************************************************************//

ExpandGroupCommand::ExpandGroupCommand(panda::PandaDocument* document,
									   panda::Group* group,
									   QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_document(document)
	, m_group(group)
{
	setText(QCoreApplication::translate("ExpandGroupCommand", "expand group"));
}

void ExpandGroupCommand::redo()
{
	// If the group was selected, select all objects inside it
	if(m_document->isSelected(m_group))
	{
		for(auto object : m_group->getObjects())
			m_document->selectionAdd(object.data());
	}
}

void ExpandGroupCommand::undo()
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

//***************************************************************//

GroupAddObjectCommand::GroupAddObjectCommand(panda::Group* group,
											 QSharedPointer<panda::PandaObject> object,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_group(group)
	, m_object(object)
{
	setText(QCoreApplication::translate("GroupAddObjectCommand", "add object to group"));
}

void GroupAddObjectCommand::redo()
{
	m_group->addObject(m_object);
}

void GroupAddObjectCommand::undo()
{
	m_group->removeObject(m_object.data());
}

//***************************************************************//

GroupRemoveObjectCommand::GroupRemoveObjectCommand(panda::Group* group,
											 QSharedPointer<panda::PandaObject> object,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_group(group)
	, m_object(object)
{
	setText(QCoreApplication::translate("GroupRemoveObjectCommand", "remove object from group"));
}

void GroupRemoveObjectCommand::redo()
{
	m_group->removeObject(m_object.data());
}

void GroupRemoveObjectCommand::undo()
{
	m_group->addObject(m_object);
}
