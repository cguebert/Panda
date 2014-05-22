#include <QUndoCommand>
#include <QCoreApplication>

#include <panda/PandaObject.h>
#include <ui/command/LinkDatasCommand.h>

LinkDatasCommand::LinkDatasCommand(panda::BaseData* targetData,
								   panda::BaseData* parentData,
								   QUndoCommand* parent)
	: QUndoCommand(parent)
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

	setText(QCoreApplication::translate("LinkDatasCommand", "change data link"));
}

void LinkDatasCommand::redo()
{
	for(auto link : m_links)
	{
		panda::BaseData* target = link.m_targetObject->getData(link.m_targetDataName);
		panda::BaseData* parent = nullptr;
		if(link.m_newParentObject)
			parent = link.m_newParentObject->getData(link.m_newParentDataName);
		target->getOwner()->dataSetParent(target, parent);
	}
}

void LinkDatasCommand::undo()
{
	for(auto link : m_links)
	{
		panda::BaseData* target = link.m_targetObject->getData(link.m_targetDataName);
		panda::BaseData* parent = nullptr;
		if(link.m_initialParentObject)
			parent = link.m_initialParentObject->getData(link.m_initialParentDataName);
		target->getOwner()->dataSetParent(target, parent);
	}
}
/*
bool LinkDatasCommand::mergeWith(const QUndoCommand *other)
{
	const LinkDatasCommand* command = dynamic_cast<const LinkDatasCommand*>(other);
	if(!command)
		return false;

	m_links += command->m_links;
	return true;
}
*/
