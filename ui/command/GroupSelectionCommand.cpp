#include <ui/command/GroupSelectionCommand.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/ObjectsSelection.h>

#include <panda/object/Group.h>

SelectGroupCommand::SelectGroupCommand(GraphView* view, panda::Group* group)
	: m_view(view)
	, m_group(group)
{
	setText("select group");
}

void SelectGroupCommand::redo()
{
	// If at least one of the object inside the group was selected, select the group
	auto& selection = m_view->selection();
	for(auto& object : m_group->getObjects())
	{
		if(selection.isSelected(object.get()))
		{
			selection.add(m_group);
			return;
		}
	}
}

void SelectGroupCommand::undo()
{
	// If the group was selected, select all objects inside it
	auto& selection = m_view->selection();
	if(selection.isSelected(m_group))
	{
		for(auto& object : m_group->getObjects())
			selection.add(object.get());
	}
}

//****************************************************************************//

SelectObjectsInGroupCommand::SelectObjectsInGroupCommand(GraphView* view, panda::Group* group)
	: m_view(view)
	, m_group(group)
{
	setText("select objects in group");
}

void SelectObjectsInGroupCommand::redo()
{
	// If the group was selected, select all objects inside it
	auto& selection = m_view->selection();
	if(selection.isSelected(m_group))
	{
		for(auto& object : m_group->getObjects())
			selection.add(object.get());
	}
}

void SelectObjectsInGroupCommand::undo()
{
	// If at least one of the object inside the group was selected, select the group
	auto& selection = m_view->selection();
	for(auto& object : m_group->getObjects())
	{
		if(selection.isSelected(object.get()))
		{
			selection.add(m_group);
			return;
		}
	}
}
