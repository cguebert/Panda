#include <panda/Dockable.h>
#include <panda/PandaDocument.h>

#include <ui/GraphView.h>

#include <QPainter>

namespace panda
{

DockObject::DockObject(QObject *parent)
	: PandaObject(parent)
{
}

DockObject::~DockObject()
{
	DockObject* defaultDock = NULL;
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());

	DockablesIterator iter(dockedObjects);
	while(iter.hasNext())
	{
		DockableObject* dockable = iter.next();
		removeInput((DataNode*)dockable);

		if(doc)
			defaultDock = dockable->getDefaultDock(doc);

		if(defaultDock == this)
			defaultDock = NULL;

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
		dockedObjects.append(dockable);
	else
		dockedObjects.insert(index, dockable);
	emit modified(this);
}

void DockObject::removeDockable(DockableObject* dockable)
{
	removeInput((DataNode*)dockable);
}

DockObject::DockablesIterator DockObject::getDockablesIterator() const
{
	return DockablesIterator(dockedObjects);
}

int DockObject::getIndexOfDockable(DockableObject* dockable) const
{
	return dockedObjects.indexOf(dockable);
}

void DockObject::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);

	if(dockedObjects.contains((DockableObject*)node))
	{
		dockedObjects.removeAll((DockableObject*)node);
		setDirtyValue();
		emit modified(this);
	}
}

//******************************************************************************************

DockableObject::DockableObject(QObject *parent)
	: PandaObject(parent)
{
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent);
	if(doc)
	{
		DockObject* dock = getDefaultDock(doc);
		if(dock)
			dock->addDockable(this);
	}
}

void DockableObject::setParentDock(DockObject* dock)
{
	parentDock = dock;
}

DockObject* DockableObject::getParentDock()
{
	return parentDock.data();
}

DockObject* DockableObject::getDefaultDock(PandaDocument* /*doc*/)
{
	return NULL;
}

} // namespace panda
