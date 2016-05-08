#ifndef MOVELAYERCOMMAND_H
#define MOVELAYERCOMMAND_H

#include <panda/core.h>
#include <panda/UndoStack.h>

namespace panda
{

class ObjectsList;
class PandaObject;

class PANDA_CORE_API MoveLayerCommand : public UndoCommand
{
public:
	MoveLayerCommand(ObjectsList& objectsList, PandaObject* layer, int pos);

	virtual void redo();
	virtual void undo();

protected:
	ObjectsList& m_objectsList;
	PandaObject* m_layer;
	int m_prevPos, m_newPos;
};

} // namespace panda

#endif
