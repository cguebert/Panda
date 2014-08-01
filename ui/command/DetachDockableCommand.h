#ifndef DETACHDOCKABLECOMMAND_H
#define DETACHDOCKABLECOMMAND_H

#include <QUndoCommand>

namespace panda
{
class DockObject;
class DockableObject;
}

class DetachDockableCommand : public QUndoCommand
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

class AttachDockableCommand : public QUndoCommand
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

class ReorderDockableCommand : public QUndoCommand
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
