#ifndef REMOVEGENERICDATACOMMAND_H
#define REMOVEGENERICDATACOMMAND_H

#include <QUndoCommand>

namespace panda
{
class DockObject;
class DockableObject;
}

class DetachDockableCommand : public QUndoCommand
{
public:
	DetachDockableCommand(panda::DockObject* dock, panda::DockableObject* dockable, int index, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::DockObject* m_dock;
	panda::DockableObject* m_dockable;
	int m_index;
};

#endif
