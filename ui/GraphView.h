#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QMap>
#include <QTextStream>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
class Layer;
class Renderer;
}

class ObjectDrawStruct;
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

    static bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2);

    void resetView();

    ObjectDrawStruct* getObjectDrawStruct(panda::PandaObject* obj);
    QRectF getDataRect(panda::BaseData* data);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    panda::PandaObject* getObjectAtPos(const QPointF& pt);
    void moveView(const QPointF& delta);

    void addLinkTag(panda::BaseData* input, panda::BaseData* output);
    void removeLinkTag(panda::BaseData* input, panda::BaseData* output);

signals:
    void modified();
    void showStatusBarMessage(QString);

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
    void savingObject(QDataStream&, panda::PandaObject*);
    void savingObject(QTextStream&, panda::PandaObject*);
    void loadingObject(QDataStream&, panda::PandaObject*);
    void loadingObject(QTextStream&, panda::PandaObject*);
    int getAvailableLinkTagIndex();
    void updateLinkTags(bool reset=false);

private:
    panda::PandaDocument* pandaDocument;
    int zoomLevel;
    qreal zoomFactor;
    QPointF viewDelta;
    QPointF previousMousePos, currentMousePos;
	enum MovingAction { MOVING_NONE=0, MOVING_START, MOVING_OBJECT, MOVING_VIEW, MOVING_SELECTION, MOVING_LINK, MOVING_ZOOM };
    MovingAction movingAction;
    panda::BaseData *clickedData, *hoverData;
    QMap<panda::PandaObject*, QSharedPointer<ObjectDrawStruct> > objectDrawStructs;
    QMap<panda::BaseData*, QSharedPointer<LinkTag> > linkTags;
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
