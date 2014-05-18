#include <QUndoCommand>
#include <QCoreApplication>

#include <ui/GraphView.h>
#include <ui/command/MoveObjectCommand.h>

MoveObjectCommand::MoveObjectCommand(GraphView* view, QList<panda::PandaObject*> objects,
									 QPointF delta, QUndoCommand* parent)
	: QUndoCommand(parent)
	, m_view(view)
	, m_objects(objects)
	, m_delta(delta)
{
	setText(QCoreApplication::translate("MoveObjectCommand", "move objects"));
}

void MoveObjectCommand::redo()
{
	m_view->moveObjects(m_objects, m_delta);
}

void MoveObjectCommand::undo()
{
	m_view->moveObjects(m_objects, -m_delta);
}

bool MoveObjectCommand::mergeWith(const QUndoCommand *other)
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
