#ifndef MOVEOBJECTCOMMAND_H
#define MOVEOBJECTCOMMAND_H

#include <panda/UndoStack.h>
#include <panda/types/Point.h>

namespace panda
{
	
class PandaObject;

namespace graphview {
	class ObjectPositionAddon;
}

class PANDA_CORE_API MoveObjectCommand : public UndoCommand
{
public:
	MoveObjectCommand(PandaObject* object, types::Point delta);
	MoveObjectCommand(std::vector<PandaObject*> objects, types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const UndoCommand *other);

protected:
	void getPositionAddons();
	void moveObjects(types::Point delta);

	std::vector<PandaObject*> m_objects;
	std::vector<graphview::ObjectPositionAddon*> m_positionAddons;
	types::Point m_delta;
};

} // namespace panda

#endif
