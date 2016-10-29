#pragma once

#include <ui/graphview/graphics/DrawColors.h>
#include <panda/messaging.h>

#include <memory>
#include <vector>

namespace panda
{
class DockableObject;
class DockObject;
class ObjectsList;
class PandaDocument;
class PandaObject;
}

namespace graphview
{

namespace object {
	class ObjectRenderer;
}

namespace graphics {
	class DrawList;
}

class LinksList;
class LinkTagsList;
class ObjectRenderersList;
class ObjectsSelection;
class Viewport;
class ViewGui;
class ViewInteraction;
class ViewRenderer;

class GraphView
{
public:
	explicit GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList);
	virtual ~GraphView();

	panda::PandaDocument* document() const;

	ObjectsSelection& selection() const;
	LinksList& linksList() const;
	LinkTagsList& linkTagsList() const;
	panda::ObjectsList& objectsList() const;
	ObjectRenderersList& objectRenderers() const;
	Viewport& viewport() const;
	ViewInteraction& interaction() const;
	ViewGui& gui() const;

	const std::vector<object::ObjectRenderer*>& selectedObjectsRenderers() const;
	bool isLoading() const;

	void emitViewportModified();

	/// Objects docked to the default docks are sorted by their height in the graph view
	void sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock);
	void sortDockablesInDock(panda::DockObject* dock);
	void sortAllDockables();

	void setGui(const std::shared_ptr<ViewRenderer>& viewRenderer, std::unique_ptr<ViewGui> viewGui);

	void beforeDraw();
	virtual void initializeRenderer(ViewRenderer& viewRenderer);
	virtual void drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors);

	void objectsMoved(); // Refresh the view, links & tags
	void update(); // Ask to redraw the view

	panda::msg::Signal<void()> updateNeeded;
	panda::msg::Signal<void()> modified;
	panda::msg::Signal<void()> viewportModified;

	void modifiedObject(panda::PandaObject* object);

protected:
	void addedObject(panda::PandaObject* object);
	void removeObject(panda::PandaObject* object);

	void startLoading();
	void loadingFinished();

	void changedDock(panda::DockableObject* dockable);

	void updateDirtyRenderers();

	void selectionChanged();
	void objectsReordered();

	void initComponents();

	panda::PandaDocument* m_pandaDocument;
	panda::ObjectsList& m_objectsList;

	std::shared_ptr<ViewRenderer> m_viewRenderer;
	std::unique_ptr<ViewGui> m_viewGui;

	bool m_isLoading = false; /// We don't update the view while loading (unnecessary events)

	panda::msg::Observer m_observer; /// Used to connect to signals (and disconnect automatically on destruction)

	std::vector<object::ObjectRenderer*> m_selectedObjectsRenderers; /// The renderers for the selected objects

	std::shared_ptr<graphics::DrawList> m_drawList;
	bool m_objectsMoved = false;

	std::unique_ptr<LinksList> m_linksList;
	std::unique_ptr<LinkTagsList> m_linkTagsList;
	std::unique_ptr<ObjectRenderersList> m_objectRenderersList;
	std::unique_ptr<ObjectsSelection> m_objectsSelection;
	std::unique_ptr<Viewport> m_viewport;
	std::unique_ptr<ViewInteraction> m_interaction;
};

//****************************************************************************//

inline ObjectsSelection& GraphView::selection() const
{ return *m_objectsSelection; }

inline LinksList& GraphView::linksList() const
{ return *m_linksList; }

inline LinkTagsList& GraphView::linkTagsList() const
{ return *m_linkTagsList; }

inline panda::ObjectsList& GraphView::objectsList() const
{ return m_objectsList; }

inline ObjectRenderersList& GraphView::objectRenderers() const
{ return *m_objectRenderersList; }

inline Viewport& GraphView::viewport() const
{ return *m_viewport; }

inline ViewInteraction& GraphView::interaction() const
{ return *m_interaction; }

inline ViewGui& GraphView::gui() const
{ return *m_viewGui; }

inline panda::PandaDocument* GraphView::document() const
{ return m_pandaDocument; }

inline const std::vector<object::ObjectRenderer*>& GraphView::selectedObjectsRenderers() const
{ return m_selectedObjectsRenderers; }

inline bool GraphView::isLoading() const
{ return m_isLoading; }

} // namespace graphview
