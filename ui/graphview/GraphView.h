#pragma once

#include <QOpenGLWidget>

#include <functional>
#include <map>
#include <memory>
#include <set>

#include <ui/custom/ScrollContainer.h>
#include <ui/graphview/graphics/DrawList.h>
#include <ui/graphview/graphics/DrawColors.h>
#include <panda/messaging.h>

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

class LinksList;
class LinkTag;
class LinkTagsList;
class ObjectRenderersList;
class ObjectsSelection;
class Viewport;
class ViewGUI;
class ViewInteraction;
class ViewRenderer;

class GraphView : public QOpenGLWidget, public ScrollableView
{
	Q_OBJECT

public:
	explicit GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);
	~GraphView();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	panda::PandaDocument* document() const;

	panda::types::Point getNewObjectPosition();

	void objectsMoved(); // Refresh the view, links & tags

	/// Objects docked to the default docks are sorted by their height in the graph view
	void sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock);
	void sortDockablesInDock(panda::DockObject* dock);
	void sortAllDockables();

	ObjectsSelection& selection() const;
	ViewRenderer& viewRenderer() const;
	LinksList& linksList() const;
	LinkTagsList& linkTagsList() const;
	panda::ObjectsList& objectsList() const;
	ObjectRenderersList& objectRenderers() const;
	Viewport& viewport() const;
	ViewInteraction& interaction() const;
	ViewGUI& gui() const;

	// From ScrollableView
	virtual QSize viewSize() override;
	virtual QPoint viewPosition() override;
	virtual void scrollView(QPoint position) override;

	void executeNextRefresh(std::function<void ()> func);

	const std::vector<object::ObjectRenderer*>& selectedObjectsRenderers() const;
	bool isLoading() const;

	panda::types::Rect contentsArea() const; // Should return the visible area of the view
	panda::types::Point toScreen(const panda::types::Point& pos) const; // Transform from the position in the view to the position in the screen

	void emitViewportModified();

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
	void focusOutEvent(QFocusEvent*) override;

#ifdef PANDA_LOG_EVENTS
	void paintLogDebug(graphics::DrawList& list, graphics::DrawColors& colors);
#endif
	void paintDirtyState(graphics::DrawList& list, graphics::DrawColors& colors);

	void updateDirtyRenderers();

	void selectionChanged();
	void objectsReordered();

signals:
	void modified();
	void showStatusBarMessage(QString);
	void viewportModified();
	void lostFocus(QWidget*);

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

protected:
	panda::PandaDocument* m_pandaDocument;
	panda::ObjectsList& m_objectsList;

	panda::types::Rect m_objectsRect; /// Area taken by the objects on the screen
	panda::types::Rect m_viewRect; /// Area taken by the objects on the screen, including zoom

	bool m_isLoading = false; /// We don't update the view while loading (unnecessary events)

	panda::msg::Observer m_observer; /// Used to connect to signals (and disconnect automatically on destruction)

	std::unique_ptr<ObjectsSelection> m_objectsSelection; /// Contains the selected objects and the corresponding signals
	std::vector<object::ObjectRenderer*> m_selectedObjectsRenderers; /// The renderers for the selected objects

	bool m_debugDirtyState = false;

	std::unique_ptr<ViewRenderer> m_viewRenderer; /// Custom OpenGL drawing
	graphics::DrawColors m_drawColors; /// So that we aquire Qt colors only once
	bool m_objectsMoved = false;

	std::vector<std::function<void ()>> m_functionsToExecuteNextRefresh;

	std::unique_ptr<LinksList> m_linksList;
	std::unique_ptr<LinkTagsList> m_linkTagsList;
	std::unique_ptr<ObjectRenderersList> m_objectRenderersList;
	std::unique_ptr<Viewport> m_viewport;
	std::unique_ptr<ViewGUI> m_viewGUI;
	std::unique_ptr<ViewInteraction> m_interaction;
};

//****************************************************************************//

inline ObjectsSelection& GraphView::selection() const
{ return *m_objectsSelection; }

inline ViewRenderer& GraphView::viewRenderer() const
{ return *m_viewRenderer; }

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

inline ViewGUI& GraphView::gui() const
{ return *m_viewGUI; }

inline void GraphView::debugDirtyState(bool show)
{ m_debugDirtyState = show; update(); }

inline panda::PandaDocument* GraphView::document() const
{ return m_pandaDocument; }

inline const std::vector<object::ObjectRenderer*>& GraphView::selectedObjectsRenderers() const
{ return m_selectedObjectsRenderers; }

inline bool GraphView::isLoading() const
{ return m_isLoading; }

} // namespace graphview
