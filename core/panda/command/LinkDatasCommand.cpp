#include <panda/object/PandaObject.h>
#include <panda/command/CommandId.h>
#include <panda/command/LinkDatasCommand.h>

LinkDatasCommand::LinkDatasCommand(panda::BaseData* targetData,
								   panda::BaseData* parentData)
{
	LinkStruct link;
	link.m_targetObject = targetData->getOwner();
	link.m_targetDataName = targetData->getName();
	if(parentData)
	{
		link.m_newParentObject = parentData->getOwner();
		link.m_newParentDataName = parentData->getName();
	}
	else
		link.m_newParentObject = nullptr;

	// Save current parent
	if(targetData->getParent())
	{
		auto initial = targetData->getParent();
		link.m_initialParentObject = initial->getOwner();
		link.m_initialParentDataName = initial->getName();
	}
	else
		link.m_initialParentObject = nullptr;
	m_links.push_back(link);

	setText("change data link");
}

int LinkDatasCommand::id() const
{
	return getCommandId<LinkDatasCommand>();
}

void LinkDatasCommand::redo()
{
	for(auto link : m_links)
	{
		panda::BaseData* target = link.m_targetObject->getData(link.m_targetDataName);
		if(!target)
			continue;
		panda::BaseData* parent = nullptr;
		if(link.m_newParentObject)
			parent = link.m_newParentObject->getData(link.m_newParentDataName);
		target->getOwner()->dataSetParent(target, parent);
	}

	// Use childs if present
	UndoCommand::redo();
}

void LinkDatasCommand::undo()
{
	// Use childs if present
	UndoCommand::undo();

	for(auto link : m_links)
	{
		panda::BaseData* target = link.m_targetObject->getData(link.m_targetDataName);
		if(!target)
			continue;
		panda::BaseData* parent = nullptr;
		if(link.m_initialParentObject)
			parent = link.m_initialParentObject->getData(link.m_initialParentDataName);
		target->getOwner()->dataSetParent(target, parent);
	}
}
/*
bool LinkDatasCommand::mergeWith(const panda::UndoCommand* other)
{
	const LinkDatasCommand* command = dynamic_cast<const LinkDatasCommand*>(other);
	if(!command)
		return false;

	m_links += command->m_links;
	return true;
}
*/
