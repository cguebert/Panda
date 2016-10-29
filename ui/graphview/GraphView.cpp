#include <ui/graphview/GraphView.h>

#include <ui/graphview/object/DockableRenderer.h>
#include <ui/graphview/LinksList.h>
#include <ui/graphview/LinkTagsList.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/ViewGUI.h>
#include <ui/graphview/ViewInteraction.h>
#include <ui/graphview/Viewport.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/command/DockableCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>


using Point = panda::types::Point;
using Rect = panda::types::Rect;

namespace graphview
{

GraphView::GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList)
	: m_pandaDocument(doc)
	, m_objectsList(objectsList)
{
}

GraphView::~GraphView() = default;

void GraphView::initComponents()
{
	if (!m_linksList)           m_linksList           = std::make_unique<LinksList>(*this);
	if (!m_linkTagsList)        m_linkTagsList        = std::make_unique<LinkTagsList>(*this);
	if (!m_objectsSelection)    m_objectsSelection    = std::make_unique<ObjectsSelection>(objectsList());
	if (!m_objectRenderersList) m_objectRenderersList = std::make_unique<ObjectRenderersList>();
	if (!m_viewport)            m_viewport            = std::make_unique<Viewport>(*this); 
	if (!m_interaction)         m_interaction         = std::make_unique<ViewInteraction>(*this);

	m_observer.get(selection().selectionChanged).connect<GraphView, &GraphView::selectionChanged>(this);

	auto& docSignals = m_pandaDocument->getSignals();
	m_observer.get(docSignals.modified).connect<GraphView, &GraphView::update>(this);
	m_observer.get(docSignals.modifiedObject).connect<GraphView, &GraphView::modifiedObject>(this);
	m_observer.get(docSignals.startLoading).connect<GraphView, &GraphView::startLoading>(this);
	m_observer.get(docSignals.loadingFinished).connect<GraphView, &GraphView::loadingFinished>(this);
	m_observer.get(docSignals.changedDock).connect<GraphView, &GraphView::changedDock>(this);

	m_observer.get(m_objectsList.addedObject).connect<GraphView, &GraphView::addedObject>(this);
	m_observer.get(m_objectsList.removedObject).connect<GraphView, &GraphView::removeObject>(this);
	m_observer.get(m_objectsList.reorderedObjects).connect<GraphView, &GraphView::objectsReordered>(this);

	m_observer.get(viewport().modified).connect<GraphView, &GraphView::update>(this);
	m_observer.get(viewport().modified).connect<GraphView, &GraphView::emitViewportModified>(this);
}

void GraphView::beforeDraw()
{
	interaction().beforeDraw();

	updateDirtyRenderers();

	if (m_objectsMoved)
	{
		m_objectsMoved = false;
		modified.run();
		linkTagsList().setDirty();
		linksList().clear();
		viewport().updateObjectsRect();
	}
}

void GraphView::initializeRenderer(ViewRenderer& viewRenderer)
{
	m_drawList = std::make_shared<graphics::DrawList>(viewRenderer);
	interaction().initializeRenderer(viewRenderer);
	linksList().initializeRenderer(viewRenderer);
	objectRenderers().initializeRenderer(viewRenderer);
}

void GraphView::drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors)
{
	const auto displayRect = viewport().displayRect();

	linkTagsList().onBeginDraw();
	linksList().onBeginDraw(drawColors);
	interaction().onBeginDraw(drawColors);

	auto& drawList = *m_drawList;
	drawList.clear();

	const auto& orderedObjectRenderers = objectRenderers().getOrdered();

	// Give a possibility to draw behind normal objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->drawBackground(drawList, drawColors);
	}

	// Draw links
	drawList.merge(*linksList().linksDrawList());

	// Draw the objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->draw(drawList, drawColors);
	}

	// Give a possibility to draw in front of normal objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->drawForeground(drawList, drawColors);
	}

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& objRnd : m_selectedObjectsRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->draw(drawList, drawColors, true);
	}

	linkTagsList().drawTags(drawList, drawColors);
	interaction().drawInteraction(drawList, drawColors);

	// Add the main draw list
	viewRenderer.addDrawList(m_drawList);

	// Highlight connected Datas
	if(interaction().highlightConnectedDatas())
		viewRenderer.addDrawList(interaction().connectedDatasDrawList());
}

void GraphView::addedObject(panda::PandaObject* object)
{
	// Creating a Renderer depending on the class of the object being added
	auto objRnd = objectRenderers().get(object);
	if (!objRnd)
	{
		auto objRndPtr = object::ObjectRendererFactory::getInstance()->createRenderer(this, object);
		
		if (m_viewRenderer->initialized())
			objRndPtr->initializeRenderer(*m_viewRenderer);

		objectRenderers().set(object, objRndPtr);
		objRnd = objRndPtr.get();
	}

	objRnd->setDirty();
	update();
}

void GraphView::removeObject(panda::PandaObject* object)
{
	objectRenderers().remove(object);
	interaction().removeObject(object);
	linkTagsList().clear();
	linksList().clear();

	viewport().updateObjectsRect();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
	linksList().clear();
	auto objRnd = objectRenderers().get(object);
	if(objRnd)	// Can be called before the object is fully created
	{
		objRnd->setDirty();

		panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object);
		if (dock)
		{
			auto dobjRnd = dynamic_cast<object::DockObjectRenderer*>(objRnd);
			if (dobjRnd)
				dobjRnd->placeDockableObjects();
		}

		update();
	}
}

void GraphView::objectsMoved()
{
	m_objectsMoved = true;
	update();
}

void GraphView::update()
{
	updateNeeded.run();
}

void GraphView::sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock)
{
	int prevIndex = defaultDock->getIndexOfDockable(dockable);
	auto dockables = defaultDock->getDockedObjects();

	std::sort(dockables.begin(), dockables.end(), [this](panda::DockableObject* lhs, panda::DockableObject* rhs){
		auto lpos = objectRenderers().get(lhs)->getPosition();
		auto rpos = objectRenderers().get(rhs)->getPosition();
		if(lpos.y == rpos.y)
			return lpos.x > rpos.x;
		return lpos.y < rpos.y;
	});

	auto iter = std::find(dockables.begin(), dockables.end(), dockable);
	if(iter != dockables.end())
	{
		int newIndex = iter - dockables.begin();
		if(newIndex == prevIndex)
			return;

		m_pandaDocument->getUndoStack().push(std::make_shared<panda::ReorderDockableCommand>(defaultDock, dockable, newIndex));
	}
}

void GraphView::sortDockablesInDock(panda::DockObject* dock)
{
	for(const auto dockable : dock->getDockedObjects())
	{
		auto defaultDock = dockable->getDefaultDock();
		auto parentDock = dockable->getParentDock();
		if(defaultDock && parentDock == defaultDock)
			sortDockable(dockable, defaultDock);
	}
}

void GraphView::sortAllDockables()
{
	for(const auto& object : m_objectsList.get())
	{
		const auto dockable = dynamic_cast<panda::DockableObject*>(object.get());
		if(!dockable)
			continue;
		auto defaultDock = dockable->getDefaultDock();
		auto parentDock = dockable->getParentDock();
		if(defaultDock && parentDock == defaultDock)
			sortDockable(dockable, defaultDock);
	}
}

void GraphView::startLoading()
{
	m_isLoading = true;
}

void GraphView::loadingFinished()
{
	m_isLoading = false;
	sortAllDockables();
	updateDirtyRenderers();
}

void GraphView::changedDock(panda::DockableObject* dockable)
{
	auto defaultDock = dockable->getDefaultDock();
	auto parentDock = dockable->getParentDock();
	if(defaultDock && parentDock == defaultDock)
		sortDockable(dockable, defaultDock);
}

void GraphView::selectionChanged()
{
	m_selectedObjectsRenderers = objectRenderers().get(selection().get());
	update();
}

void GraphView::updateDirtyRenderers()
{
	bool updated = false;

	// Bugfix: update the dock objects last
	const auto& orderedObjectRenderers = objectRenderers().getOrdered();
	for (auto objRnd : orderedObjectRenderers)
		if(!dynamic_cast<panda::DockObject*>(objRnd->getObject()))
			updated |= objRnd->updateIfDirty();

	for (auto objRnd : orderedObjectRenderers)
		if(dynamic_cast<panda::DockObject*>(objRnd->getObject()))
			updated |= objRnd->updateIfDirty();

	if (!updated)
		return;

	linksList().clear();
	linkTagsList().clear();
	interaction().clear();
	viewport().updateObjectsRect();
}

void GraphView::objectsReordered()
{
	objectRenderers().reorder(m_objectsList);
}

void GraphView::emitViewportModified()
{
	viewportModified.run();
}

void GraphView::setGui(const std::shared_ptr<ViewRenderer>& viewRenderer, std::unique_ptr<ViewGui> viewGui)
{
	m_viewRenderer = viewRenderer;
	m_viewGui = std::move(viewGui);

	// Create the draw structs for the objects already present
	for (const auto& object : m_objectsList.get())
		addedObject(object.get());

	updateDirtyRenderers();
	viewport().updateObjectsRect();
}

} // namespace graphview
