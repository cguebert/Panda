#ifndef DOCKABLEDRAWSTRUCT_H
#define DOCKABLEDRAWSTRUCT_H

#include <panda/Dockable.h>

#include <ui/drawstruct/ObjectDrawStruct.h>

class DockObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockObjectDrawStruct(GraphView* view, panda::DockObject* m_object);

	virtual void drawShape(QPainter* painter);
	virtual void drawText(QPainter* painter);
	virtual QSize getObjectSize();
	virtual void move(const QPointF& delta);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);

	int getDockableIndex(const QRectF& rect);

	static const int dockEmptyRendererHeight = 30;
	static const int dockHoleWidth = 80;
	static const int dockHoleMargin = 2;
	static const int dockRendererMargin = 10;
protected:
	panda::DockObject* dockObject;
	QPainterPath shapePath;
	QList<int> dockablesY;	// Used only in "getDockableIndex"
};

class DockableObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockableObjectDrawStruct(GraphView* view, panda::DockableObject* m_object);

	virtual void drawShape(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);

	static const int dockableCircleWidth = 20;
protected:
	QPainterPath shapePath;
};

#endif
