#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QMap>
#include <ui/ObjectDrawStruct.h>
#include <QDomDocument>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
class Layer;
class Renderer;
}

class LinkTag;

class GraphView : public QWidget
{
	Q_OBJECT

public:
	explicit GraphView(panda::PandaDocument* doc, QWidget *parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	QPointF getViewDelta() const;
	const panda::PandaDocument* getDocument() const;
	const panda::BaseData* getClickedData() const;
	const panda::BaseData* getContextMenuData() const;

	static bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2);

	void resetView();

	ObjectDrawStruct* getObjectDrawStruct(panda::PandaObject* obj);
	QRectF getDataRect(panda::BaseData* data);

	int getAvailableLinkTagIndex();

	enum ContextMenuReason {
		MENU_OBJECT = 0x1,
		MENU_DATA = 0x2,
		MENU_LINK = 0x4
	};

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

private:
	panda::PandaDocument* pandaDocument;
	int zoomLevel;
	qreal zoomFactor;
	QPointF viewDelta;
	QPointF previousMousePos, currentMousePos;
	enum MovingAction { MOVING_NONE=0, MOVING_START, MOVING_OBJECT, MOVING_VIEW, MOVING_SELECTION, MOVING_LINK, MOVING_ZOOM };
	MovingAction movingAction;
	panda::BaseData *clickedData, *hoverData, *contextMenuData;
	QMap<panda::PandaObject*, QSharedPointer<ObjectDrawStruct> > objectDrawStructs;
	QMap<panda::BaseData*, QSharedPointer<LinkTag> > linkTags;
	bool recomputeTags;
};

class LinkTag
{
public:
	LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output);
	void addOutput(panda::BaseData* output);
	void removeOutput(panda::BaseData* output);
	void update();
	bool isEmpty();
	bool isHovering(const QPointF& point);

	void moveView(const QPointF& delta);
	void draw(QPainter* painter);

	panda::BaseData* getInputData();

	int index;
	bool hovering;

	static const int tagW = 20;
	static const int tagH = 11;
	static const int tagMargin = 10;
protected:
	GraphView* parentView;
	panda::BaseData* inputData;
	QRectF inputDataRect;
	QMap<panda::BaseData*, QRectF> outputDatas;
};

#endif
