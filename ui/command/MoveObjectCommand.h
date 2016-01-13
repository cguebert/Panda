#ifndef MOVEOBJECTCOMMAND_H
#define MOVEOBJECTCOMMAND_H

#include <panda/UndoStack.h>
#include <QPointF>

#include <vector>

namespace panda
{
class PandaObject;
}

class GraphView;

class MoveObjectCommand : public panda::UndoCommand
{
public:
	MoveObjectCommand(GraphView* view, panda::PandaObject* object, QPointF delta);
	MoveObjectCommand(GraphView* view, std::vector<panda::PandaObject*> objects, QPointF delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	GraphView* m_view;
	std::vector<panda::PandaObject*> m_objects;
	QPointF m_delta;
};

#endif
