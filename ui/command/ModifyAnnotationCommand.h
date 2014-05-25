#ifndef MODIFYANNOTATIONCOMMAND_H
#define MODIFYANNOTATIONCOMMAND_H

#include <QUndoCommand>
#include <QPointF>

class AnnotationDrawStruct;

class MoveAnnotationTextCommand : public QUndoCommand
{
public:
	MoveAnnotationTextCommand(AnnotationDrawStruct* drawStruct, QPointF delta, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	AnnotationDrawStruct* m_drawStruct;
	QPointF m_delta;
};

//***************************************************************//

class MoveAnnotationEndCommand : public QUndoCommand
{
public:
	MoveAnnotationEndCommand(AnnotationDrawStruct* drawStruct, QPointF delta, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	AnnotationDrawStruct* m_drawStruct;
	QPointF m_delta;
};

#endif
