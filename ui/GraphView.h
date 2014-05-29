#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QDomDocument>
#include <QMap>
#include <set>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
class Layer;
class Renderer;
class ScopedMacro;
}

class LinkTag;
class QStylePainter;
class ObjectDrawStruct;

class GraphView : public QWidget
{
	Q_OBJECT

public:
	explicit GraphView(panda::PandaDocument* doc, QWidget *parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	QPointF getViewDelta() const;
	panda::PandaDocument* getDocument() const;
	const panda::BaseData* getClickedData() const;
	const panda::BaseData* getContextMenuData() const;

	static bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2);

	void resetView();

	ObjectDrawStruct* getObjectDrawStruct(panda::PandaObject* object);
	QSharedPointer<ObjectDrawStruct> getSharedObjectDrawStruct(panda::PandaObject* object);
	void setObjectDrawStruct(panda::PandaObject* object, QSharedPointer<ObjectDrawStruct> drawStruct);

	QRectF getDataRect(panda::BaseData* data);

	int getAvailableLinkTagIndex();

	qreal getZoom();

	enum ContextMenuReason {
		MENU_OBJECT = 0x1,
		MENU_DATA = 0x2,
		MENU_LINK = 0x4
	};

	void moveObjects(QList<panda::PandaObject*> objects, QPointF delta);

	void setRecomputeTags(); /// Same as calling updateLinkTags, but it does it next redraw

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);

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

signals:
	void modified();
	void showStatusBarMessage(QString);
	void showContextMenu(QPoint pos, int reason);

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
	void savingObject(QDomDocument&, QDomElement&, panda::PandaObject*);
	void loadingObject(QDomElement&, panda::PandaObject*);
	void updateLinkTags(bool reset=false);
	void removeLink();
	void hoverDataInfo();

private:
	panda::PandaDocument* m_pandaDocument;

	int m_zoomLevel;
	qreal m_zoomFactor;
	QPointF m_viewDelta;
	QPointF m_previousMousePos, m_currentMousePos;

	enum MovingAction { MOVING_NONE=0, MOVING_START, MOVING_OBJECT, MOVING_VIEW, MOVING_SELECTION, MOVING_LINK, MOVING_ZOOM, MOVING_CUSTOM };
	MovingAction m_movingAction;

	panda::BaseData *m_clickedData, *m_hoverData, *m_contextMenuData;

	QMap<panda::PandaObject*, QSharedPointer<ObjectDrawStruct> > m_objectDrawStructs;
	ObjectDrawStruct* m_capturedDrawStruct; /// Clicked ObjectDrawStruct that want to intercept mouse events

	QMap<panda::BaseData*, QSharedPointer<LinkTag> > m_linkTags;
	bool m_recomputeTags; /// Should we recompute the linkTags next PaintEvent?

	QTimer* m_hoverTimer; /// Counting how long the mouse is staying over a Data
	bool m_highlightConnectedDatas;

	bool m_useMagneticSnap; /// Do we help align objects when moving them with the mouse?
	std::set<qreal> m_snapTargetsX, m_snapTargetsY;
	QPointF m_snapDelta;

	QList<panda::PandaObject*> m_customSelection; /// Objects on which the current action is applied

	QSharedPointer<panda::ScopedMacro> m_moveObjectsMacro;
};

#endif
