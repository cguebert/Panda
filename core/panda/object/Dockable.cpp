#include <panda/object/Dockable.h>
#include <panda/PandaDocument.h>
#include <panda/command/DockableCommand.h>
#include <panda/helper/algorithm.h>

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
		removeInput(*docked);

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
	if (helper::contains(m_dockedObjects, dockable))
		reorderDockable(dockable, index);
	else
	{
		dockable->setParentDock(this);
		addInput(*dockable);
		if (index < 0)
			m_dockedObjects.push_back(dockable);
		else
			m_dockedObjects.insert(m_dockedObjects.begin() + index, dockable);
		parentDocument()->onModifiedObject(this);
	}
}

void DockObject::doRemoveInput(DataNode& node)
{
	DataNode::doRemoveInput(node);

	const auto iter = std::find(m_dockedObjects.begin(), m_dockedObjects.end(), &node);
	if(iter != m_dockedObjects.end())
	{
		m_dockedObjects.erase(iter);
		setDirtyValue(this);
		parentDocument()->onModifiedObject(this);
	}
}

int DockObject::getIndexOfDockable(DockableObject* dockable) const
{
	const auto iter = std::find(m_dockedObjects.begin(), m_dockedObjects.end(), dockable);
	if(iter != m_dockedObjects.end())
		return iter - m_dockedObjects.begin();
	return -1;
}

void DockObject::reorderDockable(DockableObject* dockable, int index)
{
	const auto iter = std::find(m_dockedObjects.begin(), m_dockedObjects.end(), (DockableObject*)dockable);
	if(iter != m_dockedObjects.end())
	{
		const int oldIndex = iter - m_dockedObjects.begin();
		if(oldIndex == index)
			return;

		m_dockedObjects.erase(iter);
		if(index < 0)
			m_dockedObjects.push_back(dockable);
		else
			m_dockedObjects.insert(m_dockedObjects.begin() + index, dockable);

		setDirtyValue(this);
		parentDocument()->onModifiedObject(this);
	}
}

void DockObject::removedFromDocument()
{
	auto& undoStack = parentDocument()->getUndoStack();
	if(undoStack.isInCommandMacro())
	{
		auto docked = m_dockedObjects;
		for(auto it = docked.rbegin(); it != docked.rend(); ++it)
		{
			auto dockable = *it;
			undoStack.push(std::make_shared<DetachDockableCommand>(this, dockable));
			auto defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				undoStack.push(std::make_shared<AttachDockableCommand>(defaultDock, dockable, 0));
			parentDocument()->onChangedDock(dockable);
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
	auto& undoStack = parentDocument()->getUndoStack();
	if(m_parentDock && undoStack.isInCommandMacro())
		undoStack.push(std::make_shared<DetachDockableCommand>(m_parentDock, this));
}

} // namespace panda
