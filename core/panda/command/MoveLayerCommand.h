#ifndef MOVELAYERCOMMAND_H
#define MOVELAYERCOMMAND_H

#include <panda/core.h>
#include <panda/UndoStack.h>

namespace panda
{

class PandaDocument;
class PandaObject;

class PANDA_CORE_API MoveLayerCommand : public UndoCommand
{
public:
	MoveLayerCommand(PandaDocument* document, PandaObject* layer, int pos);

	virtual void redo();
	virtual void undo();

protected:
	PandaDocument* m_document;
	PandaObject* m_layer;
	int m_prevPos, m_newPos;
};

} // namespace panda

#endif
