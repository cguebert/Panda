#ifndef MODIFYANNOTATIONCOMMAND_H
#define MODIFYANNOTATIONCOMMAND_H

#include <panda/UndoStack.h>
#include <QPointF>

class AnnotationDrawStruct;

class MoveAnnotationTextCommand : public panda::UndoCommand
{
public:
	MoveAnnotationTextCommand(AnnotationDrawStruct* drawStruct, QPointF delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	AnnotationDrawStruct* m_drawStruct;
	QPointF m_delta;
};

//****************************************************************************//

class MoveAnnotationEndCommand : public panda::UndoCommand
{
public:
	MoveAnnotationEndCommand(AnnotationDrawStruct* drawStruct, QPointF delta);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const panda::UndoCommand *other);

protected:
	AnnotationDrawStruct* m_drawStruct;
	QPointF m_delta;
};

#endif
