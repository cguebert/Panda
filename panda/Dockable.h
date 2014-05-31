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

	typedef QList<DockableObject*> DockablesList;
	virtual const DockablesList& getDockables() const;

	virtual int getIndexOfDockable(DockableObject* dockable) const;

protected:
	virtual void doRemoveInput(DataNode* node);

	DockablesList m_dockedObjects;
};

//****************************************************************************//

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

//****************************************************************************//

inline void DockObject::removeDockable(DockableObject* dockable)
{ removeInput((DataNode*)dockable); }

inline const DockObject::DockablesList& DockObject::getDockables() const
{ return m_dockedObjects; }

inline int DockObject::getIndexOfDockable(DockableObject* dockable) const
{ return m_dockedObjects.indexOf(dockable); }

inline bool DockObject::accepts(DockableObject* /*dockable*/) const
{ return true; }

inline void DockableObject::setParentDock(DockObject* dock)
{ m_parentDock = dock; }

inline DockObject* DockableObject::getParentDock()
{ return m_parentDock; }

inline DockObject* DockableObject::getDefaultDock()
{ return nullptr; }

} // namespace panda

#endif
