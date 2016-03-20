#include <panda/command/CommandId.h>
#include <ui/drawstruct/AnnotationDrawStruct.h>
#include <ui/command/ModifyAnnotationCommand.h>

MoveAnnotationTextCommand::MoveAnnotationTextCommand(AnnotationDrawStruct* drawStruct,
													 QPointF delta)
	: m_drawStruct(drawStruct)
	, m_delta(delta)
{
	setText("move annotation");
}

int MoveAnnotationTextCommand::id() const
{
	return panda::getCommandId<MoveAnnotationTextCommand>();
}

void MoveAnnotationTextCommand::redo()
{
	m_drawStruct->moveText(m_delta);
}

void MoveAnnotationTextCommand::undo()
{
	m_drawStruct->moveText(-m_delta);
}

bool MoveAnnotationTextCommand::mergeWith(const panda::UndoCommand *other)
{
	const MoveAnnotationTextCommand* command = dynamic_cast<const MoveAnnotationTextCommand*>(other);
	if(!command)
		return false;
	if(m_drawStruct == command->m_drawStruct)
	{
		m_delta += command->m_delta;
		return true;
	}

	return false;
}

//****************************************************************************//

MoveAnnotationEndCommand::MoveAnnotationEndCommand(AnnotationDrawStruct* drawStruct,
												   QPointF delta)
	: m_drawStruct(drawStruct)
	, m_delta(delta)
{
	setText("modify annotation");
}

int MoveAnnotationEndCommand::id() const
{
	return panda::getCommandId<MoveAnnotationEndCommand>();
}

void MoveAnnotationEndCommand::redo()
{
	m_drawStruct->moveEnd(m_delta);
}

void MoveAnnotationEndCommand::undo()
{
	m_drawStruct->moveEnd(-m_delta);
}

bool MoveAnnotationEndCommand::mergeWith(const panda::UndoCommand *other)
{
	const MoveAnnotationEndCommand* command = dynamic_cast<const MoveAnnotationEndCommand*>(other);
	if(!command)
		return false;
	if(m_drawStruct == command->m_drawStruct)
	{
		m_delta += command->m_delta;
		return true;
	}

	return false;
}
