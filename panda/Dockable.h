#ifndef DOCKABLE_H
#define DOCKABLE_H

#include <panda/PandaObject.h>

#include <QList>
#include <QPointer>

namespace panda
{

class DockableObject;

class DockObject : public PandaObject
{
public:
	DockObject(QObject *parent);
	virtual ~DockObject();

	virtual bool accepts(DockableObject* dockable) const;
	virtual void addDockable(DockableObject* dockable, int index = -1);
	virtual void removeDockable(DockableObject* dockable);

	typedef QListIterator<DockableObject*> DockablesIterator;
	virtual DockablesIterator getDockablesIterator() const;

	virtual int getIndexOfDockable(DockableObject* dockable) const;

protected:
	virtual void doRemoveInput(DataNode* node);

	QList<DockableObject*> dockedObjects;
};

class DockableObject : public PandaObject
{
public:
	DockableObject(QObject *parent);
	virtual void setParentDock(DockObject* dock);
	virtual DockObject* getParentDock();
	virtual DockObject* getDefaultDock(PandaDocument* doc);

protected:
	QPointer<DockObject> parentDock;
};

} // namespace panda

#endif
