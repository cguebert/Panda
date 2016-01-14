#ifndef DOCKABLE_H
#define DOCKABLE_H

#include <panda/object/PandaObject.h>

#include <vector>

namespace panda
{

class DockableObject;

class PANDA_CORE_API DockObject : public PandaObject
{
public:
	PANDA_CLASS(DockObject, PandaObject)

	explicit DockObject(PandaDocument* document);
	virtual ~DockObject();

	virtual bool accepts(DockableObject* dockable) const;
	virtual void addDockable(DockableObject* dockable, int index = -1);
	virtual void removeDockable(DockableObject* dockable);

	typedef std::vector<DockableObject*> DockablesList;
	virtual const DockablesList& getDockedObjects() const;

	virtual int getIndexOfDockable(DockableObject* dockable) const;
	virtual void reorderDockable(DockableObject* dockable, int index);

	virtual void removedFromDocument();

protected:
	virtual void doRemoveInput(DataNode& node) override;

	DockablesList m_dockedObjects;
};

//****************************************************************************//

class PANDA_CORE_API DockableObject : public PandaObject
{
public:
	PANDA_CLASS(DockableObject, PandaObject)

	explicit DockableObject(PandaDocument* document);

	virtual void postCreate();
	virtual void setParentDock(DockObject* dock);
	virtual DockObject* getParentDock() const;
	virtual DockObject* getDefaultDock() const;

	virtual void removedFromDocument();

protected:
	DockObject* m_parentDock;
};

//****************************************************************************//

inline void DockObject::removeDockable(DockableObject* dockable)
{ removeInput(*(DataNode*)dockable); }

inline const DockObject::DockablesList& DockObject::getDockedObjects() const
{ return m_dockedObjects; }

inline bool DockObject::accepts(DockableObject* /*dockable*/) const
{ return true; }

inline void DockableObject::setParentDock(DockObject* dock)
{ m_parentDock = dock; }

inline DockObject* DockableObject::getParentDock() const
{ return m_parentDock; }

inline DockObject* DockableObject::getDefaultDock() const
{ return nullptr; }

} // namespace panda

#endif
