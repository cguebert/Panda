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

class CreateGroupCommand : public QUndoCommand
{
public:
	CreateGroupCommand(panda::PandaDocument* document, panda::Group* group, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//***************************************************************//

class ExpandGroupCommand : public QUndoCommand
{
public:
	ExpandGroupCommand(panda::PandaDocument* document, panda::Group* group, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//***************************************************************//

class GroupAddObjectCommand : public QUndoCommand
{
public:
	GroupAddObjectCommand(panda::Group* group, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QSharedPointer<panda::PandaObject> m_object;
};

//***************************************************************//

class GroupRemoveObjectCommand : public QUndoCommand
{
public:
	GroupRemoveObjectCommand(panda::Group* group, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QSharedPointer<panda::PandaObject> m_object;
};

#endif
