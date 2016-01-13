#ifndef REMOVEGENERICDATACOMMAND_H
#define REMOVEGENERICDATACOMMAND_H

#include <panda/core.h>
#include <panda/UndoStack.h>

namespace panda
{
class GenericObject;
}

class PANDA_CORE_API RemoveGenericDataCommand : public panda::UndoCommand
{
public:
	RemoveGenericDataCommand(panda::GenericObject* object, int type, int index);

	virtual void redo();
	virtual void undo();

protected:
	panda::GenericObject* m_object;
	int m_type, m_index;
};

#endif
