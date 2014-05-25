#ifndef ADDOBJECTCOMMAND_H
#define ADDOBJECTCOMMAND_H

#include <QUndoCommand>
#include <QSharedPointer>
#include <QVector>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class GraphView;
class ObjectDrawStruct;

class AddObjectCommand : public QUndoCommand
{
public:
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, QVector<QSharedPointer<panda::PandaObject>> objects, QUndoCommand* parent = nullptr);
	AddObjectCommand(panda::PandaDocument* document, GraphView* view, QList<panda::PandaObject*> objects, QUndoCommand* parent = nullptr);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

	virtual bool mergeWith(const QUndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	bool m_ignoreRedo;
	QVector<QSharedPointer<panda::PandaObject>> m_objects;
	QVector<QSharedPointer<ObjectDrawStruct>> m_drawStructs;
};

#endif
