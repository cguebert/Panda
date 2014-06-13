#include <panda/Dockable.h>
#include <panda/PandaDocument.h>

#include <ui/command/DetachDockableCommand.h>

namespace panda
{

DockObject::DockObject(PandaDocument* document)
	: PandaObject(document)
{
}

DockObject::~DockObject()
{
	DockObject* defaultDock = nullptr;

	auto dockedObjects = getDockedObjects();
	for(auto docked : dockedObjects)
	{
		removeInput((DataNode*)docked);

		defaultDock = docked->getDefaultDock();

		if(defaultDock == this)
			defaultDock = nullptr;

		docked->setParentDock(defaultDock);

		if(defaultDock)
			defaultDock->addDockable(docked);
	}
}

void DockObject::addDockable(DockableObject* dockable, int index)
{
	dockable->setParentDock(this);
	addInput((DataNode*)dockable);
	if(index < 0)
		m_dockedObjects.push_back(dockable);
	else
		m_dockedObjects.insert(m_dockedObjects.begin() + index, dockable);
	m_parentDocument->onModifiedObject(this);
}

void DockObject::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);

	auto iter = std::find(m_dockedObjects.begin(), m_dockedObjects.end(), (DockableObject*)node);
	if(iter != m_dockedObjects.end())
	{
		m_dockedObjects.erase(iter);
		setDirtyValue(this);
		m_parentDocument->onModifiedObject(this);
	}
}

int DockObject::getIndexOfDockable(DockableObject* dockable) const
{
	auto iter = std::find(m_dockedObjects.begin(), m_dockedObjects.end(), dockable);
	if(iter != m_dockedObjects.end())
		return iter - m_dockedObjects.begin();
	return -1;
}

void DockObject::removedFromDocument()
{
	if(m_parentDocument->isInCommandMacro())
	{
		auto docked = m_dockedObjects;
		for(auto it = docked.rbegin(); it != docked.rend(); ++it)
		{
			m_parentDocument->addCommand(new DetachDockableCommand(this, *it));
			m_parentDocument->addCommand(new AttachDockableCommand((*it)->getDefaultDock(), *it, 0));
		}
	}
}

//****************************************************************************//

DockableObject::DockableObject(PandaDocument* document)
	: PandaObject(document)
{
}

void DockableObject::postCreate()
{
	PandaObject::postCreate();
	DockObject* dock = getDefaultDock();
	if(dock)
		dock->addDockable(this);
}

void DockableObject::removedFromDocument()
{
	if(m_parentDock && m_parentDocument->isInCommandMacro())
		m_parentDocument->addCommand(new DetachDockableCommand(m_parentDock, this));
}

} // namespace panda
