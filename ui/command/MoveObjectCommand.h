#ifndef MOVEOBJECTCOMMAND_H
#define MOVEOBJECTCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QPointF>

namespace panda
{
class PandaObject;
}

class GraphView;

class MoveObjectCommand : public QUndoCommand
{
public:
	MoveObjectCommand(GraphView* view, panda::PandaObject* object, QPointF delta, QUndoCommand* parent = nullptr);
	MoveObjectCommand(GraphView* view, QList<panda::PandaObject*> objects, QPointF delta, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	GraphView* m_view;
	QList<panda::PandaObject*> m_objects;
	QPointF m_delta;
};

#endif
