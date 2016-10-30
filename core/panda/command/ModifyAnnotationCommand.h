#ifndef MODIFYANNOTATIONCOMMAND_H
#define MODIFYANNOTATIONCOMMAND_H

#include <panda/UndoStack.h>
#include <panda/types/Point.h>

namespace panda 
{

class Annotation;

class PANDA_CORE_API MoveAnnotationEndCommand : public UndoCommand
{
public:
	MoveAnnotationEndCommand(Annotation* annotation, types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const UndoCommand *other);

protected:
	Annotation* m_annotation;
	types::Point m_delta;
};

} // namespace panda

#endif
