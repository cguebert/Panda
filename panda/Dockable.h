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
	PANDA_CLASS(DockObject, PandaObject)

	explicit DockObject(PandaDocument* document);
	virtual ~DockObject();

	virtual bool accepts(DockableObject* dockable) const;
	virtual void addDockable(DockableObject* dockable, int index = -1);
	virtual void removeDockable(DockableObject* dockable);

	typedef QListIterator<DockableObject*> DockablesIterator;
	virtual DockablesIterator getDockablesIterator() const;

	virtual int getIndexOfDockable(DockableObject* dockable) const;

protected:
	virtual void doRemoveInput(DataNode* node);

	QList<DockableObject*> m_dockedObjects;
};

class DockableObject : public PandaObject
{
public:
	PANDA_CLASS(DockableObject, PandaObject)

	explicit DockableObject(PandaDocument* document);

	virtual void postCreate();
	virtual void setParentDock(DockObject* dock);
	virtual DockObject* getParentDock();
	virtual DockObject* getDefaultDock();

	virtual void removedFromDocument();

protected:
	DockObject* m_parentDock;
};

} // namespace panda

#endif
