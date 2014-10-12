#ifndef MOVELAYERCOMMAND_H
#define MOVELAYERCOMMAND_H

#include <QUndoCommand>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class MoveLayerCommand : public QUndoCommand
{
public:
	MoveLayerCommand(panda::PandaDocument* document, panda::PandaObject* layer, int pos, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::PandaObject* m_layer;
	int m_prevPos, m_newPos;
};

#endif
