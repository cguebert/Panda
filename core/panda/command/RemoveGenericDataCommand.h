#ifndef REMOVEGENERICDATACOMMAND_H
#define REMOVEGENERICDATACOMMAND_H

#include <panda/core.h>
#include <QUndoCommand>

namespace panda
{
class GenericObject;
}

class PANDA_CORE_API RemoveGenericDataCommand : public QUndoCommand
{
public:
	RemoveGenericDataCommand(panda::GenericObject* object, int type, int index, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::GenericObject* m_object;
	int m_type, m_index;
};

#endif
