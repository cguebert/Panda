#pragma once

#include <QOpenGLWidget>

#include <functional>
#include <map>
#include <memory>
#include <set>

#include <ui/custom/ScrollContainer.h>
#include <ui/graphview/graphics/DrawColors.h>
#include <panda/messaging.h>

class MainWindow;

namespace panda
{
class BaseData;
class DockableObject;
class DockObject;
class Layer;
class ObjectsList;
class PandaDocument;
class PandaObject;
class Renderer;
class ScopedMacro;
class XmlElement;
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
class LinkTag;
class LinkTagsList;
class ObjectRenderersList;
class ObjectsSelection;
class Viewport;
class ViewGui;
class ViewInteraction;
class ViewRenderer;

class GraphView : public QOpenGLWidget, public ScrollableView
{
	Q_OBJECT

public:
	explicit GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow);
	virtual ~GraphView();

	panda::PandaDocument* document() const;

	void objectsMoved(); // Refresh the view, links & tags

	/// Objects docked to the default docks are sorted by their height in the graph view
	void sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock);
	void sortDockablesInDock(panda::DockObject* dock);
	void sortAllDockables();

	ObjectsSelection& selection() const;
	LinksList& linksList() const;
	LinkTagsList& linkTagsList() const;
	panda::ObjectsList& objectsList() const;
	ObjectRenderersList& objectRenderers() const;
	Viewport& viewport() const;
	ViewInteraction& interaction() const;
	ViewGui& gui() const;

	void executeNextRefresh(std::function<void ()> func);

	const std::vector<object::ObjectRenderer*>& selectedObjectsRenderers() const;
	bool isLoading() const;

	void emitViewportModified();

	void setGui(const std::shared_ptr<ViewRenderer>& viewRenderer, std::unique_ptr<ViewGui> viewGui);

	void beforeDraw();
	virtual void initializeRenderer(ViewRenderer& viewRenderer);
	virtual void drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors);

public slots:
	void copy();
	void cut();
	void paste();
	void del();
	void addedObject(panda::PandaObject* object);
	void removeObject(panda::PandaObject* object);
	void modifiedObject(panda::PandaObject* object);
	void startLoading();
	void loadingFinished();
	void changedDock(panda::DockableObject* dockable);
	void showChooseWidgetDialog();
	void debugDirtyState(bool show = true);
	void setDataLabel();

signals:
	void modified();
	void viewportModified();
	void lostFocus(QWidget*);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
	void focusOutEvent(QFocusEvent*) override;

	// From ScrollableView
	QSize viewSize() override;
	QPoint viewPosition() override;
	void scrollView(QPoint position) override;

#ifdef PANDA_LOG_EVENTS
	void paintLogDebug(graphics::DrawList& list, graphics::DrawColors& colors);
#endif
	void paintDirtyState(graphics::DrawList& list, graphics::DrawColors& colors);

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

	bool m_debugDirtyState = false;

	std::shared_ptr<graphics::DrawList> m_drawList;
	graphics::DrawColors m_drawColors; /// So that we aquire Qt colors only once
	bool m_objectsMoved = false;

	std::vector<std::function<void ()>> m_functionsToExecuteNextRefresh;

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

inline void GraphView::debugDirtyState(bool show)
{ m_debugDirtyState = show; update(); }

inline panda::PandaDocument* GraphView::document() const
{ return m_pandaDocument; }

inline const std::vector<object::ObjectRenderer*>& GraphView::selectedObjectsRenderers() const
{ return m_selectedObjectsRenderers; }

inline bool GraphView::isLoading() const
{ return m_isLoading; }

} // namespace graphview
