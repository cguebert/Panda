#ifndef CREATEOBJECTCOMMAND_H
#define CREATEOBJECTCOMMAND_H

#include <QUndoCommand>
#include <QSharedPointer>
#include <QVector>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class CreateObjectCommand : public QUndoCommand
{
public:
	CreateObjectCommand(panda::PandaDocument* document, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual int id() const { return 2; }

	virtual void redo();
	virtual void undo();

//	virtual bool mergeWith(const QUndoCommand *other);

protected:
	panda::PandaDocument* m_document;
	QVector<QSharedPointer<panda::PandaObject>> m_objects;
};

#endif
