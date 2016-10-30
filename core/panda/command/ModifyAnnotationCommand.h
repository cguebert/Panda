#ifndef MODIFYANNOTATIONCOMMAND_H
#define MODIFYANNOTATIONCOMMAND_H

#include <panda/UndoStack.h>
#include <panda/types/Point.h>

namespace panda 
{
	class Annotation;
}

class PANDA_CORE_API MoveAnnotationEndCommand : public panda::UndoCommand
{
public:
	MoveAnnotationEndCommand(panda::Annotation* annotation, panda::types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	panda::Annotation* m_annotation;
	panda::types::Point m_delta;
};

#endif
