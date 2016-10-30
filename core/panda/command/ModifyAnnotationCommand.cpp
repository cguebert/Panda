#include <panda/command/ModifyAnnotationCommand.h>
#include <panda/command/CommandId.h>
#include <panda/object/Annotation.h>

namespace panda
{

MoveAnnotationEndCommand::MoveAnnotationEndCommand(Annotation* annotation,
												   types::Point delta)
	: m_annotation(annotation)
	, m_delta(delta)
{
	setText("modify annotation");
}

int MoveAnnotationEndCommand::id() const
{
	return getCommandId<MoveAnnotationEndCommand>();
}

void MoveAnnotationEndCommand::redo()
{
	m_annotation->setDeltaToEnd(m_annotation->getDeltaToEnd() + m_delta);
}

void MoveAnnotationEndCommand::undo()
{
	m_annotation->setDeltaToEnd(m_annotation->getDeltaToEnd() - m_delta);
}

bool MoveAnnotationEndCommand::mergeWith(const UndoCommand *other)
{
	const MoveAnnotationEndCommand* command = dynamic_cast<const MoveAnnotationEndCommand*>(other);
	if(!command)
		return false;
	if(m_annotation == command->m_annotation)
	{
		m_delta += command->m_delta;
		return true;
	}

	return false;
}

} // namespace panda
