#ifndef MOVEOBJECTCOMMAND_H
#define MOVEOBJECTCOMMAND_H

#include <panda/UndoStack.h>
#include <panda/types/Point.h>

#include <vector>

namespace panda {
class PandaObject;
}

namespace graphview {
	class ViewPositionAddon;
}

class MoveObjectCommand : public panda::UndoCommand
{
public:
	MoveObjectCommand(panda::PandaObject* object, panda::types::Point delta);
	MoveObjectCommand(std::vector<panda::PandaObject*> objects, panda::types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	void getPositionAddons();
	void moveObjects(panda::types::Point delta);

	std::vector<panda::PandaObject*> m_objects;
	std::vector<graphview::ViewPositionAddon*> m_positionAddons;
	panda::types::Point m_delta;
};

#endif
