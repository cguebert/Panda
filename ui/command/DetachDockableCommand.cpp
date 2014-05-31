#include <QCoreApplication>

#include <panda/Dockable.h>
#include <ui/command/DetachDockableCommand.h>

DetachDockableCommand::DetachDockableCommand(panda::DockObject* dock,
											 panda::DockableObject* dockable,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_dock(dock)
	, m_dockable(dockable)
{
	m_index = dock->getIndexOfDockable(dockable);
	setText(QCoreApplication::translate("DetachDockableCommand", "detach dockable object"));
}

void DetachDockableCommand::redo()
{
	m_dock->removeDockable(m_dockable);
}

void DetachDockableCommand::undo()
{
	m_dock->addDockable(m_dockable, m_index);
}

//****************************************************************************//

AttachDockableCommand::AttachDockableCommand(panda::DockObject* dock,
											 panda::DockableObject* dockable,
											 int index,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_dock(dock)
	, m_dockable(dockable)
	, m_index(index)
{
	setText(QCoreApplication::translate("AttachDockableCommand", "attach dockable object"));
}

void AttachDockableCommand::redo()
{
	m_dock->addDockable(m_dockable, m_index);
}

void AttachDockableCommand::undo()
{
	m_dock->removeInput(m_dockable);
}
