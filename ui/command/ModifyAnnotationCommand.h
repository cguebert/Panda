#ifndef MODIFYANNOTATIONCOMMAND_H
#define MODIFYANNOTATIONCOMMAND_H

#include <panda/UndoStack.h>
#include <QPointF>

class AnnotationDrawStruct;

class MoveAnnotationTextCommand : public panda::UndoCommand
{
public:
	MoveAnnotationTextCommand(AnnotationDrawStruct* drawStruct, panda::types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	AnnotationDrawStruct* m_drawStruct;
	panda::types::Point m_delta;
};

//****************************************************************************//

class MoveAnnotationEndCommand : public panda::UndoCommand
{
public:
	MoveAnnotationEndCommand(AnnotationDrawStruct* drawStruct, panda::types::Point delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	AnnotationDrawStruct* m_drawStruct;
	panda::types::Point m_delta;
};

#endif
