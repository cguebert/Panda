#include <QCoreApplication>

#include <panda/Dockable.h>
#include <ui/command/DetachDockableCommand.h>

DetachDockableCommand::DetachDockableCommand(panda::DockObject* dock,
											 panda::DockableObject* dockable,
											 int index,
											 QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_dock(dock)
	, m_dockable(dockable)
	, m_index(index)
{

	setText(QCoreApplication::translate("DetachDockableCommand", "detach dockable object"));
}

void DetachDockableCommand::redo()
{
	m_dock->removeInput(m_dockable);
}

void DetachDockableCommand::undo()
{
	m_dock->addDockable(m_dockable, m_index);
}
