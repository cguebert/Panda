#include <ui/GraphView.h>
#include <panda/command/CommandId.h>
#include <ui/command/MoveObjectCommand.h>

MoveObjectCommand::MoveObjectCommand(GraphView* view,
									 panda::PandaObject* object,
									 QPointF delta)
	: m_view(view)
	, m_delta(delta)
{
	m_objects.push_back(object);
	setText("move objects");
}

MoveObjectCommand::MoveObjectCommand(GraphView* view,
									 std::vector<panda::PandaObject*> objects,
									 QPointF delta)
	: m_view(view)
	, m_objects(objects)
	, m_delta(delta)
{
	setText("move objects");
}

int MoveObjectCommand::id() const
{
	return getCommandId<MoveObjectCommand>();
}

void MoveObjectCommand::redo()
{
	m_view->moveObjects(m_objects, m_delta);
}

void MoveObjectCommand::undo()
{
	m_view->moveObjects(m_objects, -m_delta);
}

bool MoveObjectCommand::mergeWith(const panda::UndoCommand *other)
{
	const MoveObjectCommand* command = dynamic_cast<const MoveObjectCommand*>(other);
	if(!command)
		return false;
	if(m_view == command->m_view && m_objects == command->m_objects)
	{
		m_delta += command->m_delta;
		return true;
	}

	return false;
}
