#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QOpenGLWidget>

#include <map>
#include <memory>
#include <set>

#include <ui/custom/ScrollContainer.h>
#include <ui/graphview/graphics/DrawList.h>
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

class LinkTag;
class ObjectDrawStruct;
class ObjectsSelection;
class ViewRenderer;

struct DrawColors
{
// Can be modified to pass current object info
	unsigned int penColor = 0;
	unsigned int fillColor = 0;
	float penWidth = 1.0f;
	
// Other colors
	unsigned int midLightColor = 0;
	unsigned int lightColor = 0;
	unsigned int highlightColor = 0;
};

class GraphView : public QOpenGLWidget, public ScrollableView
{
	Q_OBJECT

public:
	using ObjectDrawStructPtr = std::shared_ptr<ObjectDrawStruct>;

	explicit GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);
	~GraphView();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	panda::types::Point getViewDelta() const;
	panda::PandaDocument* getDocument() const;
	const panda::BaseData* getClickedData() const;
	const panda::BaseData* getContextMenuData() const;

	bool canLinkWith(const panda::BaseData* data) const; /// Is it possible to link this data and the clicked data

	void resetView();

	ObjectDrawStructPtr getSharedObjectDrawStruct(panda::PandaObject* object);
	ObjectDrawStruct* getObjectDrawStruct(panda::PandaObject* object);
	std::vector<ObjectDrawStruct*> getObjectDrawStructs(const std::vector<panda::PandaObject*>& objects);
	void setObjectDrawStruct(panda::PandaObject* object, const ObjectDrawStructPtr& drawStruct);

	panda::types::Point getNewObjectPosition();

	int getAvailableLinkTagIndex();

	float getZoom();

	enum ContextMenuReason {
		MENU_OBJECT	    = 1 << 0,
		MENU_DATA	    = 1 << 1,
		MENU_LINK	    = 1 << 2,
		MENU_IMAGE	    = 1 << 3,
		MENU_TAG	    = 1 << 4
	};

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
	void paintLogDebug(DrawList& list, DrawColors& colors);
#endif
	void paintDirtyState(DrawList& list, DrawColors& colors);

	ObjectDrawStruct* getObjectDrawStructAtPos(const panda::types::Point& pt);
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

	void prepareSnapTargets(ObjectDrawStruct* selectedDrawStruct);
	void computeSnapDelta(ObjectDrawStruct* selectedDrawStruct, panda::types::Point position);

	void changeLink(panda::BaseData* target, panda::BaseData* parent);

	void updateDirtyDrawStructs();
	virtual void updateObjectsRect();
	virtual void updateViewRect();

	void selectionChanged();
	void objectsReordered();

	void computeCompatibleDatas(panda::BaseData* data);

signals:
	void modified();
	void showStatusBarMessage(QString);
	void showContextMenu(QPoint pos, int reason);
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
	void setLinkTagName();
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
		Link,
		Zoom,
		ZoomBox,
		Custom
	};
	Moving m_movingAction = Moving::None;

	panda::BaseData *m_clickedData = nullptr, *m_hoverData = nullptr, *m_contextMenuData = nullptr;
	panda::PandaObject *m_contextMenuObject = nullptr;

	std::map<panda::PandaObject*, ObjectDrawStructPtr> m_objectDrawStructs; /// The map of draw structs
	std::vector<ObjectDrawStruct*> m_orderedObjectDrawStructs; /// In the same order as the document

	ObjectDrawStruct* m_capturedDrawStruct = nullptr; /// Clicked ObjectDrawStruct that want to intercept mouse events

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
	std::vector<ObjectDrawStruct*> m_selectedObjectsDrawStructs; /// The renderers for the selected objects

	bool m_debugDirtyState = false;

	std::set<const panda::BaseData*> m_possibleLinks; /// When creating a new link, this contains all possible destinations

	std::vector<ObjectDrawStruct*> m_dirtyDrawStructs; /// The list of draw structs that need to be updated (we update them in the order we enter them in the list)
	std::set<ObjectDrawStruct*> m_dirtyDrawStructsSet; /// To ensure we do not update multiple times the same object

	std::unique_ptr<ViewRenderer> m_viewRenderer; /// Custom OpenGL drawing
	DrawList m_linksDrawList, m_connectedDrawList;
	DrawColors m_drawColors; /// So that we aquire Qt colors only once
	bool m_recomputeLinks = false, m_recomputeConnected = false, m_objectsMoved = false;

	/// To be able to add a label to any Data
	struct DataLabel
	{
		panda::PandaObject* object;
		panda::BaseData* data;
		std::string label;
	};
	std::vector<DataLabel> m_dataLabels;

	long long m_previousTime = 0;
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

#endif
