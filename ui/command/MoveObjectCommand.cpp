#include <ui/command/MoveObjectCommand.h>
#include <ui/graphview/object/ObjectPositionAddon.h>
#include <panda/command/CommandId.h>
#include <panda/object/ObjectAddons.h>
#include <panda/object/PandaObject.h>

MoveObjectCommand::MoveObjectCommand(panda::PandaObject* object,
									 panda::types::Point delta)
	: m_objects({ object })
	, m_delta(delta)
{
	setText("move objects");
	getPositionAddons();
}

MoveObjectCommand::MoveObjectCommand(std::vector<panda::PandaObject*> objects,
									 panda::types::Point delta)
	: m_objects(objects)
	, m_delta(delta)
{
	setText("move objects");
	getPositionAddons();
}

int MoveObjectCommand::id() const
{
	return panda::getCommandId<MoveObjectCommand>();
}

void MoveObjectCommand::redo()
{
	moveObjects(m_delta);
}

void MoveObjectCommand::undo()
{
	moveObjects(-m_delta);
}

bool MoveObjectCommand::mergeWith(const panda::UndoCommand *other)
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

void MoveObjectCommand::moveObjects(panda::types::Point delta)
{
	for (auto posAddon : m_positionAddons)
		posAddon->move(delta);
}
