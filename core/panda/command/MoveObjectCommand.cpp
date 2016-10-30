#include <panda/command/MoveObjectCommand.h>
#include <panda/command/CommandId.h>
#include <panda/graphview/object/ObjectPositionAddon.h>
#include <panda/object/ObjectAddons.h>
#include <panda/object/PandaObject.h>

namespace panda
{

MoveObjectCommand::MoveObjectCommand(PandaObject* object,
									 types::Point delta)
	: m_objects({ object })
	, m_delta(delta)
{
	setText("move objects");
	getPositionAddons();
}

MoveObjectCommand::MoveObjectCommand(std::vector<PandaObject*> objects,
									 types::Point delta)
	: m_objects(objects)
	, m_delta(delta)
{
	setText("move objects");
	getPositionAddons();
}

int MoveObjectCommand::id() const
{
	return getCommandId<MoveObjectCommand>();
}

void MoveObjectCommand::redo()
{
	moveObjects(m_delta);
}

void MoveObjectCommand::undo()
{
	moveObjects(-m_delta);
}

bool MoveObjectCommand::mergeWith(const UndoCommand *other)
{
	const MoveObjectCommand* command = dynamic_cast<const MoveObjectCommand*>(other);
	if(!command)
		return false;
	if(m_objects == command->m_objects)
	{
		m_delta += command->m_delta;
		return true;
	}

	return false;
}

void MoveObjectCommand::getPositionAddons()
{
	m_positionAddons.clear();
	for (auto obj : m_objects)
		m_positionAddons.push_back(&obj->addons().edit<graphview::ObjectPositionAddon>());
}

void MoveObjectCommand::moveObjects(types::Point delta)
{
	for (auto posAddon : m_positionAddons)
		posAddon->move(delta);
}

} // namespace panda
