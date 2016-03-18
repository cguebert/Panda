#pragma once

#include <memory>
#include <string>
#include <vector>

#include <panda/UndoStack.h>

namespace panda
{
class Group;
class PandaObject;
class BaseData;
}

class GraphView;

class SelectGroupCommand : public panda::UndoCommand
{
public:
	SelectGroupCommand(GraphView* view, panda::Group* group);

	virtual void redo();
	virtual void undo();

protected:
	GraphView* m_view;
	panda::Group* m_group;
};

//****************************************************************************//

class SelectObjectsInGroupCommand : public panda::UndoCommand
{
public:
	SelectObjectsInGroupCommand(GraphView* view, panda::Group* group);

	virtual void redo();
	virtual void undo();

protected:
	GraphView* m_view;
	panda::Group* m_group;
};
