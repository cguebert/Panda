#ifndef DETACHDOCKABLECOMMAND_H
#define DETACHDOCKABLECOMMAND_H

#include <panda/UndoStack.h>

namespace panda
{

class DockObject;
class DockableObject;

class PANDA_CORE_API DetachDockableCommand : public UndoCommand
{
public:
	DetachDockableCommand(DockObject* dock, DockableObject* dockable);

	virtual void redo();
	virtual void undo();

protected:
	DockObject* m_dock;
	DockableObject* m_dockable;
	int m_index;
};

//****************************************************************************//

class PANDA_CORE_API AttachDockableCommand : public UndoCommand
{
public:
	AttachDockableCommand(DockObject* dock, DockableObject* dockable, int index);

	virtual void redo();
	virtual void undo();

protected:
	DockObject* m_dock;
	DockableObject* m_dockable;
	int m_index;
};

//****************************************************************************//

class PANDA_CORE_API ReorderDockableCommand : public UndoCommand
{
public:
	ReorderDockableCommand(DockObject* dock, DockableObject* dockable, int index);

	virtual void redo();
	virtual void undo();

protected:
	DockObject* m_dock;
	DockableObject* m_dockable;
	int m_oldIndex, m_newIndex;
};

} // namespace panda

#endif
