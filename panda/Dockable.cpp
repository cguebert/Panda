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

	for(auto dockable : getDockables())
	{
		removeInput((DataNode*)dockable);

		defaultDock = dockable->getDefaultDock();

		if(defaultDock == this)
			defaultDock = nullptr;

		dockable->setParentDock(defaultDock);

		if(defaultDock)
			defaultDock->addDockable(dockable);
	}
}

void DockObject::addDockable(DockableObject* dockable, int index)
{
	dockable->setParentDock(this);
	addInput((DataNode*)dockable);
	if(index < 0)
		m_dockedObjects.push_back(dockable);
	else
		m_dockedObjects.insert(index, dockable);
	m_parentDocument->onModifiedObject(this);
}

void DockObject::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);

	if(m_dockedObjects.contains((DockableObject*)node))
	{
		m_dockedObjects.removeAll((DockableObject*)node);
		setDirtyValue(this);
		m_parentDocument->onModifiedObject(this);
	}
}

//****************************************************************************//

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

void DockableObject::removedFromDocument()
{
	if(m_parentDock && m_parentDocument->isInCommandMacro())
		m_parentDocument->addCommand(new DetachDockableCommand(m_parentDock, this));
}

} // namespace panda
