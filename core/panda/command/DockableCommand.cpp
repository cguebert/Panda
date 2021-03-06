#include <panda/object/Dockable.h>
#include <panda/command/DockableCommand.h>

namespace panda
{

DetachDockableCommand::DetachDockableCommand(DockObject* dock,
											 DockableObject* dockable)
	: m_dock(dock)
	, m_dockable(dockable)
{
	m_index = dock->getIndexOfDockable(dockable);
	setText("detach dockable object");
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

AttachDockableCommand::AttachDockableCommand(DockObject* dock,
											 DockableObject* dockable,
											 int index)
	: m_dock(dock)
	, m_dockable(dockable)
	, m_index(index)
{
	setText("attach dockable object");
}

void AttachDockableCommand::redo()
{
	m_dock->addDockable(m_dockable, m_index);
}

void AttachDockableCommand::undo()
{
	m_dock->removeDockable(m_dockable);
}

//****************************************************************************//

ReorderDockableCommand::ReorderDockableCommand(DockObject* dock,
											 DockableObject* dockable,
											 int index)
	: m_dock(dock)
	, m_dockable(dockable)
	, m_newIndex(index)
{
	m_oldIndex = dock->getIndexOfDockable(dockable);
	setText("reorder dockable object");
}

void ReorderDockableCommand::redo()
{
	m_dock->reorderDockable(m_dockable, m_newIndex);
}

void ReorderDockableCommand::undo()
{
	m_dock->reorderDockable(m_dockable, m_oldIndex);
}

} // namespace panda
