#ifndef DETACHDOCKABLECOMMAND_H
#define DETACHDOCKABLECOMMAND_H

#include <panda/core.h>
#include <panda/UndoStack.h>

namespace panda
{
class DockObject;
class DockableObject;
}

class PANDA_CORE_API DetachDockableCommand : public panda::UndoCommand
{
public:
	DetachDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_index;
};

//****************************************************************************//

class PANDA_CORE_API AttachDockableCommand : public panda::UndoCommand
{
public:
	AttachDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable, int index);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_index;
};

//****************************************************************************//

class PANDA_CORE_API ReorderDockableCommand : public panda::UndoCommand
{
public:
	ReorderDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable, int index);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_oldIndex, m_newIndex;
};

#endif
