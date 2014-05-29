#ifndef CREATEGROUPCOMMAND_H
#define CREATEGROUPCOMMAND_H

#include <QUndoCommand>
#include <QSharedPointer>

namespace panda
{
class Group;
class PandaObject;
class PandaDocument;
}

class SelectGroupCommand : public QUndoCommand
{
public:
	SelectGroupCommand(panda::PandaDocument* document, panda::Group* group, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//***************************************************************//

class SelectObjectsInGroupCommand : public QUndoCommand
{
public:
	SelectObjectsInGroupCommand(panda::PandaDocument* document, panda::Group* group, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//***************************************************************//

class AddObjectToGroupCommand : public QUndoCommand
{
public:
	AddObjectToGroupCommand(panda::Group* group, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QSharedPointer<panda::PandaObject> m_object;
};

//***************************************************************//

class RemoveObjectFromGroupCommand : public QUndoCommand
{
public:
	RemoveObjectFromGroupCommand(panda::Group* group, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QSharedPointer<panda::PandaObject> m_object;
};

#endif
