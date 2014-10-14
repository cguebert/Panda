#ifndef DETACHDOCKABLECOMMAND_H
#define DETACHDOCKABLECOMMAND_H

#include <panda/core.h>
#include <QUndoCommand>

namespace panda
{
class DockObject;
class DockableObject;
}

class PANDA_CORE_API DetachDockableCommand : public QUndoCommand
{
public:
	DetachDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_index;
};

//****************************************************************************//

class PANDA_CORE_API AttachDockableCommand : public QUndoCommand
{
public:
	AttachDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable, int index, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_index;
};

//****************************************************************************//

class PANDA_CORE_API ReorderDockableCommand : public QUndoCommand
{
public:
	ReorderDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable, int index, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_oldIndex, m_newIndex;
};

#endif
