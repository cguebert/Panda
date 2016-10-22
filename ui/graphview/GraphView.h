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

class LinkTag;
class ObjectsSelection;
class ViewRenderer;

class GraphView : public QOpenGLWidget, public ScrollableView
{
	Q_OBJECT

public:
	using ObjectRendererPtr = std::shared_ptr<object::ObjectRenderer>;

	explicit GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);
	~GraphView();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	panda::types::Point getViewDelta() const;
	panda::PandaDocument* getDocument() const;
	const panda::BaseData* getClickedData() const;
	const panda::BaseData* getContextMenuData() const;

	bool canLinkWith(const panda::BaseData* data) const; /// Is it possible to link this data and the clicked data

	ObjectRendererPtr getSharedObjectRenderer(panda::PandaObject* object);
	object::ObjectRenderer* getObjectRenderer(panda::PandaObject* object);
	std::vector<object::ObjectRenderer*> getObjectRenderers(const std::vector<panda::PandaObject*>& objects);
	void setObjectRenderer(panda::PandaObject* object, const ObjectRendererPtr& drawStruct);

	panda::types::Point getNewObjectPosition();

	int getAvailableLinkTagIndex();

	float getZoom();

	virtual void moveObjects(std::vector<panda::PandaObject*> objects, panda::types::Point delta);
	void objectsMoved(); // Refresh the view, links & tags

	/// Objects docked to the default docks are sorted by their height in the graph view
	void sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock);
	void sortDockablesInDock(panda::DockObject* dock);
	void sortAllDockables();

	ObjectsSelection& selection() const;
	ViewRenderer& renderer() const;
	panda::ObjectsList& objectsList() const;

	// From ScrollableView
	virtual QSize viewSize() override;
	virtual QPoint viewPosition() override;
	virtual void scrollView(QPoint position) override;

	virtual bool isTemporaryView() const; // If true, do not store any pointer to this view (in commands for example)

	virtual bool getDataRect(const panda::BaseData* data, panda::types::Rect& rect);

	void executeNextRefresh(std::function<void ()> func);

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

	object::ObjectRenderer* getObjectRendererAtPos(const panda::types::Point& pt);
	virtual std::pair<panda::BaseData*, panda::types::Rect> getDataAtPos(const panda::types::Point& pt);

	using Rects = std::vector<panda::types::Rect>;
	using PointsPairs = std::vector<std::pair<panda::types::Point, panda::types::Point>>;
	virtual std::pair<Rects, PointsPairs> getConnectedDatas(panda::BaseData* data);

	void moveView(const panda::types::Point& delta);
	void moveViewIfMouseOnBorder();

	void addLinkTag(panda::BaseData* input, panda::BaseData* output);
	void removeLinkTag(panda::BaseData* input, panda::BaseData* output);
	bool hasLinkTag(panda::BaseData* input, panda::BaseData* output);

	virtual void updateLinks();
	void updateConnectedDatas();
	void updateLinkTags();

	void prepareSnapTargets(object::ObjectRenderer* selectedRenderer);
	void computeSnapDelta(object::ObjectRenderer* selectedRenderer, panda::types::Point position);

	virtual bool createLink(panda::BaseData* data1, panda::BaseData* data2);
	void changeLink(panda::BaseData* target, panda::BaseData* parent); // Return true if a link was made or modified

	void updateDirtyRenderers();
	virtual void updateObjectsRect();
	virtual void updateViewRect();

	void selectionChanged();
	void objectsReordered();

	virtual bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2);
	virtual void computeCompatibleDatas(panda::BaseData* data);

	int getContextMenuFlags(const panda::types::Point& pos);

signals:
	void modified();
	void showStatusBarMessage(QString);
	void viewModified();
	void lostFocus(QWidget*);

public slots:
	void copy();
	void cut();
	void paste();
	void del();
	void zoomIn();
	void zoomOut();
	void zoomReset();
	void centerView();
	void showAll();
	void showAllSelected();
	void moveSelectedToCenter();
	void addedObject(panda::PandaObject* object);
	void removeObject(panda::PandaObject* object);
	void modifiedObject(panda::PandaObject* object);
	void removeLink();
	void hoverDataInfo();
	void startLoading();
	void loadingFinished();
	void changedDock(panda::DockableObject* dockable);
	void showChooseWidgetDialog();
	void debugDirtyState(bool show = true);
	void setDataLabel();
	void moveObjectToBack();
	void moveObjectToFront();

protected:
	panda::PandaDocument* m_pandaDocument;
	panda::ObjectsList& m_objectsList;

	int m_zoomLevel = 0, m_wheelTicks = 0;
	float m_zoomFactor = 1.0f;
	panda::types::Point m_viewDelta;
	panda::types::Point m_previousMousePos, m_currentMousePos;

	enum class Moving
	{
		None=0,
		Start,
		Object,
		View,
		Selection,
		SelectionAdd,
		SelectionRemove,
		Link,
		Zoom,
		ZoomBox,
		Custom
	};
	Moving m_movingAction = Moving::None;

	panda::BaseData *m_clickedData = nullptr, *m_hoverData = nullptr, *m_contextMenuData = nullptr;
	panda::PandaObject *m_contextMenuObject = nullptr;

	std::map<panda::PandaObject*, ObjectRendererPtr> m_objectRenderers; /// The map of draw structs
	std::vector<object::ObjectRenderer*> m_orderedObjectRenderers; /// In the same order as the document

	object::ObjectRenderer* m_capturedRenderer = nullptr; /// Clicked object::ObjectRenderer that want to intercept mouse events

	std::vector<std::shared_ptr<LinkTag>> m_linkTags;
	std::map<panda::BaseData*, LinkTag*> m_linkTagsMap; /// Input data of the link tag
	std::set<std::pair<panda::BaseData*, panda::BaseData*>> m_linkTagsDatas; /// A copy of the link tags connections
	bool m_recomputeTags = false; /// Should we recompute the linkTags next PaintEvent?
	LinkTag* m_contextLinkTag = nullptr;

	QTimer* m_hoverTimer; /// Counting how long the mouse is staying over a Data
	bool m_highlightConnectedDatas = false;

	bool m_useMagneticSnap = true; /// Do we help align objects when moving them with the mouse?
	std::set<float> m_snapTargetsY;
	panda::types::Point m_snapDelta;

	std::vector<panda::PandaObject*> m_customSelection; /// Objects on which the current action is applied

	std::shared_ptr<panda::ScopedMacro> m_moveObjectsMacro;

	panda::types::Rect m_objectsRect; /// Area taken by the objects on the screen
	panda::types::Rect m_viewRect; /// Area taken by the objects on the screen, including zoom

	bool m_isLoading = false; /// We don't update the view while loading (unnecessary events)

	panda::msg::Observer m_observer; /// Used to connect to signals (and disconnect automatically on destruction)

	std::unique_ptr<ObjectsSelection> m_objectsSelection; /// Contains the selected objects and the corresponding signals
	std::vector<object::ObjectRenderer*> m_selectedObjectsRenderers; /// The renderers for the selected objects

	bool m_debugDirtyState = false;

	std::set<const panda::BaseData*> m_possibleLinks; /// When creating a new link, this contains all possible destinations

	std::unique_ptr<ViewRenderer> m_viewRenderer; /// Custom OpenGL drawing
	graphics::DrawList m_linksDrawList, m_connectedDrawList;
	graphics::DrawColors m_drawColors; /// So that we aquire Qt colors only once
	bool m_recomputeLinks = false, m_recomputeConnected = false, m_objectsMoved = false;

	long long m_previousTime = 0;

	std::vector<std::function<void ()>> m_functionsToExecuteNextRefresh;
};

//****************************************************************************//

inline ObjectsSelection& GraphView::selection() const
{ return *m_objectsSelection; }

inline ViewRenderer& GraphView::renderer() const
{ return *m_viewRenderer; }

inline panda::ObjectsList& GraphView::objectsList() const
{ return m_objectsList; }

inline void GraphView::debugDirtyState(bool show)
{ m_debugDirtyState = show; update(); }

inline panda::types::Point GraphView::getViewDelta() const
{ return m_viewDelta; }

inline panda::PandaDocument* GraphView::getDocument() const
{ return m_pandaDocument; }

inline const panda::BaseData* GraphView::getClickedData() const
{ return m_clickedData; }

inline const panda::BaseData* GraphView::getContextMenuData() const
{ return m_contextMenuData; }

inline float GraphView::getZoom()
{ return m_zoomFactor; }

inline void GraphView::moveView(const panda::types::Point& delta)
{ m_viewDelta -= delta; }

inline bool GraphView::hasLinkTag(panda::BaseData* input, panda::BaseData* output)
{ return m_linkTagsDatas.count(std::make_pair(input, output)) != 0; }

inline bool GraphView::canLinkWith(const panda::BaseData* data) const
{ return m_possibleLinks.count(data) != 0; }

inline bool GraphView::isTemporaryView() const
{ return false; }

} // namespace graphview
