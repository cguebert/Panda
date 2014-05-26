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

	DockablesIterator iter(m_dockedObjects);
	while(iter.hasNext())
	{
		DockableObject* dockable = iter.next();
		removeInput((DataNode*)dockable);

		defaultDock = dockable->getDefaultDock();

		if(defaultDock == this)
			defaultDock = nullptr;

		dockable->setParentDock(defaultDock);

		if(defaultDock)
			defaultDock->addDockable(dockable);
	}
}

bool DockObject::accepts(DockableObject* /*dockable*/) const
{
	return true;
}

void DockObject::addDockable(DockableObject* dockable, int index)
{
	dockable->setParentDock(this);
	addInput((DataNode*)dockable);
	if(index < 0)
		m_dockedObjects.append(dockable);
	else
		m_dockedObjects.insert(index, dockable);
	m_parentDocument->onModifiedObject(this);
}

void DockObject::removeDockable(DockableObject* dockable)
{
	removeInput((DataNode*)dockable);
}

DockObject::DockablesIterator DockObject::getDockablesIterator() const
{
	return DockablesIterator(m_dockedObjects);
}

int DockObject::getIndexOfDockable(DockableObject* dockable) const
{
	return m_dockedObjects.indexOf(dockable);
}

void DockObject::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);

	if(m_dockedObjects.contains((DockableObject*)node))
	{
		m_dockedObjects.removeAll((DockableObject*)node);
		setDirtyValue();
		m_parentDocument->onModifiedObject(this);
	}
}

//******************************************************************************************

DockableObject::DockableObject(PandaDocument* document)
	: PandaObject(document)
{
}

void DockableObject::postCreate()
{
	DockObject* dock = getDefaultDock();
	if(dock)
		dock->addDockable(this);
}

void DockableObject::setParentDock(DockObject* dock)
{
	m_parentDock = dock;
}

DockObject* DockableObject::getParentDock()
{
	return m_parentDock;
}

DockObject* DockableObject::getDefaultDock()
{
	return nullptr;
}

void DockableObject::removedFromDocument()
{
	if(m_parentDocument->isInCommandMacro())
		m_parentDocument->addCommand(new DetachDockableCommand(m_parentDock, this, m_parentDock->getIndexOfDockable(this)));
}

} // namespace panda
