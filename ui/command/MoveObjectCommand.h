#ifndef MOVEOBJECTCOMMAND_H
#define MOVEOBJECTCOMMAND_H

#include <panda/UndoStack.h>
#include <panda/types/Point.h>

#include <vector>

namespace panda
{
class PandaObject;
}

class GraphView;

class MoveObjectCommand : public panda::UndoCommand
{
public:
	MoveObjectCommand(GraphView* view, panda::PandaObject* object, panda::types::Point delta);
	MoveObjectCommand(GraphView* view, std::vector<panda::PandaObject*> objects, panda::types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	GraphView* m_view;
	std::vector<panda::PandaObject*> m_objects;
	panda::types::Point m_delta;
};

#endif
