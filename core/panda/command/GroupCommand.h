#ifndef CREATEGROUPCOMMAND_H
#define CREATEGROUPCOMMAND_H

#include <panda/core.h>

#include <memory>
#include <string>
#include <vector>

#include <panda/UndoStack.h>

namespace panda
{

class BaseData;
class Group;
class PandaObject;
class PandaDocument;

class PANDA_CORE_API AddObjectToGroupCommand : public UndoCommand
{
public:
	AddObjectToGroupCommand(Group* group, std::shared_ptr<PandaObject> object);

	virtual void redo();
	virtual void undo();

protected:
	Group* m_group;
	std::shared_ptr<PandaObject> m_object;
};

//****************************************************************************//

class PANDA_CORE_API RemoveObjectFromGroupCommand : public UndoCommand
{
public:
	RemoveObjectFromGroupCommand(Group* group, std::shared_ptr<PandaObject> object);

	virtual void redo();
	virtual void undo();

protected:
	Group* m_group;
	std::shared_ptr<PandaObject> m_object;
};

//****************************************************************************//

class PANDA_CORE_API EditGroupCommand : public UndoCommand
{
public:
	struct DataInfo
	{
		BaseData* data;
		std::string name, help;
	};

	EditGroupCommand(Group* group, std::string newName, std::vector<DataInfo> newDatas);

	virtual void redo();
	virtual void undo();

protected:
	Group* m_group;
	std::string m_prevName, m_newName;
	std::vector<DataInfo> m_prevDatas, m_newDatas;
};

} // namespace panda

#endif
