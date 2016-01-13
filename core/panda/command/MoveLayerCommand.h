#ifndef MOVELAYERCOMMAND_H
#define MOVELAYERCOMMAND_H

#include <panda/core.h>
#include <panda/UndoStack.h>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class PANDA_CORE_API MoveLayerCommand : public panda::UndoCommand
{
public:
	MoveLayerCommand(panda::PandaDocument* document, panda::PandaObject* layer, int pos);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::PandaObject* m_layer;
	int m_prevPos, m_newPos;
};

#endif
