#ifndef CREATEGROUPCOMMAND_H
#define CREATEGROUPCOMMAND_H

#include <panda/core.h>

#include <memory>
#include <string>
#include <vector>

#include <panda/UndoStack.h>

namespace panda
{
class Group;
class PandaObject;
class PandaDocument;
class BaseData;
}

class PANDA_CORE_API SelectGroupCommand : public panda::UndoCommand
{
public:
	SelectGroupCommand(panda::PandaDocument* document, panda::Group* group);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//****************************************************************************//

class PANDA_CORE_API SelectObjectsInGroupCommand : public panda::UndoCommand
{
public:
	SelectObjectsInGroupCommand(panda::PandaDocument* document, panda::Group* group);

	virtual void redo();
	virtual void undo();

protected:
	panda::PandaDocument* m_document;
	panda::Group* m_group;
};

//****************************************************************************//

class PANDA_CORE_API AddObjectToGroupCommand : public panda::UndoCommand
{
public:
	AddObjectToGroupCommand(panda::Group* group, std::shared_ptr<panda::PandaObject> object);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	std::shared_ptr<panda::PandaObject> m_object;
};

//****************************************************************************//

class PANDA_CORE_API RemoveObjectFromGroupCommand : public panda::UndoCommand
{
public:
	RemoveObjectFromGroupCommand(panda::Group* group, std::shared_ptr<panda::PandaObject> object);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	std::shared_ptr<panda::PandaObject> m_object;
};

//****************************************************************************//

class PANDA_CORE_API EditGroupCommand : public panda::UndoCommand
{
public:
	struct DataInfo
	{
		panda::BaseData* data;
		std::string name, help;
	};

	EditGroupCommand(panda::Group* group, std::string newName, std::vector<DataInfo> newDatas);

	virtual void redo();
	virtual void undo();

protected:
	panda::Group* m_group;
	std::string m_prevName, m_newName;
	std::vector<DataInfo> m_prevDatas, m_newDatas;
};

#endif
