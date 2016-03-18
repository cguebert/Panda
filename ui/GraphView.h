#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>

#include <map>
#include <memory>
#include <set>

#include <ui/ScrollContainer.h>
#include <panda/messaging.h>

namespace panda
{
class BaseData;
class DockableObject;
class DockObject;
class PandaDocument;
class PandaObject;
class Layer;
class Renderer;
class ScopedMacro;
class XmlElement;
}

class LinkTag;
class ObjectDrawStruct;
class ObjectsSelection;
class QStylePainter;

class GraphView : public QWidget, public ScrollableView
{
	Q_OBJECT

public:
	explicit GraphView(panda::PandaDocument* doc, QWidget* parent = nullptr);
	~GraphView();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	QPointF getViewDelta() const;
	panda::PandaDocument* getDocument() const;
	const panda::BaseData* getClickedData() const;
	const panda::BaseData* getContextMenuData() const;

	static bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2);

	void resetView();

	ObjectDrawStruct* getObjectDrawStruct(panda::PandaObject* object);
	std::vector<ObjectDrawStruct*> getObjectDrawStructs(const std::vector<panda::PandaObject*>& objects);
	std::shared_ptr<ObjectDrawStruct> getSharedObjectDrawStruct(panda::PandaObject* object);
	void setObjectDrawStruct(panda::PandaObject* object, std::shared_ptr<ObjectDrawStruct> drawStruct);

	QRectF getDataRect(panda::BaseData* data);

	int getAvailableLinkTagIndex();

	qreal getZoom();

	enum ContextMenuReason {
		MENU_OBJECT = 0x1,
		MENU_DATA = 0x2,
		MENU_LINK = 0x4,
		MENU_IMAGE = 0x8
	};

	void moveObjects(std::vector<panda::PandaObject*> objects, QPointF delta);

	void setRecomputeTags(); /// Same as calling updateLinkTags, but it does it next redraw

	/// Objects docked to the default docks are sorted by their height in the graph view
	void sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock);
	void sortDockablesInDock(panda::DockObject* dock);
	void sortAllDockables();

	ObjectsSelection& selection() const;

	// From ScrollableView
	virtual QSize viewSize() override;
	virtual QPoint viewPosition() override;
	virtual void scrollView(QPoint position) override;

protected:
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void contextMenuEvent(QContextMenuEvent* event);

#ifdef PANDA_LOG_EVENTS
	void paintLogDebug(QPainter* painter);
#endif

	panda::PandaObject* getObjectAtPos(const QPointF& pt);
	void moveView(const QPointF& delta);

	void addLinkTag(panda::BaseData* input, panda::BaseData* output);
	void removeLinkTag(panda::BaseData* input, panda::BaseData* output);

	void drawConnectedDatas(QStylePainter* painter, panda::BaseData* sourceData);

	void prepareSnapTargets(ObjectDrawStruct* selectedDrawStruct);
	void computeSnapDelta(QPointF position);

	void changeLink(panda::BaseData* target, panda::BaseData* parent);

	void updateViewRect();
	void emitViewModified();

signals:
	void modified();
	void showStatusBarMessage(QString);
	void showContextMenu(QPoint pos, int reason);
	void viewModified();

public slots:
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
	void savingObject(panda::XmlElement&, panda::PandaObject*);
	void loadingObject(const panda::XmlElement&, panda::PandaObject*);
	void updateLinkTags(bool reset=false);
	void removeLink();
	void hoverDataInfo();
	void startLoading();
	void loadingFinished();
	void changedDock(panda::DockableObject* dockable);
	void showChooseWidgetDialog();

private:
	panda::PandaDocument* m_pandaDocument;

	int m_zoomLevel, m_wheelTicks;
	qreal m_zoomFactor;
	QPointF m_viewDelta;
	QPointF m_previousMousePos, m_currentMousePos;

	enum MovingAction { MOVING_NONE=0, MOVING_START, MOVING_OBJECT, MOVING_VIEW, MOVING_SELECTION, MOVING_LINK, MOVING_ZOOM, MOVING_CUSTOM };
	MovingAction m_movingAction;

	panda::BaseData *m_clickedData, *m_hoverData, *m_contextMenuData;

	std::map<panda::PandaObject*, std::shared_ptr<ObjectDrawStruct> > m_objectDrawStructs;
	ObjectDrawStruct* m_capturedDrawStruct; /// Clicked ObjectDrawStruct that want to intercept mouse events

	std::map<panda::BaseData*, std::shared_ptr<LinkTag> > m_linkTags;
	bool m_recomputeTags; /// Should we recompute the linkTags next PaintEvent?

	QTimer* m_hoverTimer; /// Counting how long the mouse is staying over a Data
	bool m_highlightConnectedDatas;

	bool m_useMagneticSnap; /// Do we help align objects when moving them with the mouse?
	std::set<qreal> m_snapTargetsX, m_snapTargetsY;
	QPointF m_snapDelta;

	std::vector<panda::PandaObject*> m_customSelection; /// Objects on which the current action is applied

	std::shared_ptr<panda::ScopedMacro> m_moveObjectsMacro;

	QRectF m_viewRect; /// Area taken by the objects on the screen

	bool m_isLoading; /// We don't update the view while loading (unnecessary events)

	panda::msg::Observer m_observer; /// Used to connect to signals (and disconnect automatically on destruction)

	std::unique_ptr<ObjectsSelection> m_objectsSelection; /// Contains the selected objects and the corresponding signals
};

inline ObjectsSelection& GraphView::selection() const
{ return *m_objectsSelection; }

#endif
