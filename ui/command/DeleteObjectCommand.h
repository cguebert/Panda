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
	DeleteObjectCommand(panda::PandaDocument* document, GraphView* view,
						const QList<panda::PandaObject*>& objects, bool unlinkDatas = true, QUndoCommand* parent = nullptr);
	DeleteObjectCommand(panda::PandaDocument* document, GraphView* view,
						panda::PandaObject* object, bool unlinkDatas = true, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	QVector< QPair< QSharedPointer<panda::PandaObject>, QSharedPointer<ObjectDrawStruct> > > m_objects;

	void prepareCommand(const QList<panda::PandaObject*>& objects, bool unlinkDatas);
};

#endif
