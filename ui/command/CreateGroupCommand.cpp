#include <QCoreApplication>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <ui/command/CreateGroupCommand.h>

CreateGroupCommand::CreateGroupCommand(QUndoCommand* parent)
	: QUndoCommand(parent)
{
	setText(QCoreApplication::translate("CreateGroupCommand", "detach dockable object"));
}

void CreateGroupCommand::redo()
{
}

void CreateGroupCommand::undo()
{
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
	for(auto object : m_group->getObjects())
		m_document->selectionAdd(object.data());
}

void ExpandGroupCommand::undo()
{
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
