#ifndef CREATEGROUPCOMMAND_H
#define CREATEGROUPCOMMAND_H

#include <panda/core.h>

#include <QUndoCommand>
#include <QSharedPointer>
#include <QVector>
#include <QString>

namespace panda
{
class Group;
class PandaObject;
class PandaDocument;
class BaseData;
}

class PANDA_CORE_API SelectGroupCommand : public QUndoCommand
{
public:
	SelectGroupCommand(panda::PandaDocument* document, panda::Group* group, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//****************************************************************************//

class PANDA_CORE_API SelectObjectsInGroupCommand : public QUndoCommand
{
public:
	SelectObjectsInGroupCommand(panda::PandaDocument* document, panda::Group* group, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//****************************************************************************//

class PANDA_CORE_API AddObjectToGroupCommand : public QUndoCommand
{
public:
	AddObjectToGroupCommand(panda::Group* group, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QSharedPointer<panda::PandaObject> m_object;
};

//****************************************************************************//

class PANDA_CORE_API RemoveObjectFromGroupCommand : public QUndoCommand
{
public:
	RemoveObjectFromGroupCommand(panda::Group* group, QSharedPointer<panda::PandaObject> object, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QSharedPointer<panda::PandaObject> m_object;
};

//****************************************************************************//

class PANDA_CORE_API EditGroupCommand : public QUndoCommand
{
public:
	struct DataInfo
	{
		panda::BaseData* data;
		QString name, help;
	};

	EditGroupCommand(panda::Group* group, QString newName, QVector<DataInfo> newDatas, QUndoCommand* parent = nullptr);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	QString m_prevName, m_newName;
	QVector<DataInfo> m_prevDatas, m_newDatas;
};

#endif
