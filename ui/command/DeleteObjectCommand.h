#ifndef DELETEOBJECTCOMMAND_H
#define DELETEOBJECTCOMMAND_H

#include <QUndoCommand>
#include <QSharedPointer>
#include <QVector>
#include <QPair>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class GraphView;
class ObjectDrawStruct;

class DeleteObjectCommand : public QUndoCommand
{
public:
	DeleteObjectCommand(panda::PandaDocument* document, GraphView* view, QList<panda::PandaObject*> objects, QUndoCommand* parent = nullptr);

	virtual int id() const { return 3; }

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	QVector< QPair< QSharedPointer<panda::PandaObject>, QSharedPointer<ObjectDrawStruct> > > m_objects;
};

#endif
