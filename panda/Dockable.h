#ifndef DOCKABLE_H
#define DOCKABLE_H

#include <panda/PandaObject.h>

#include <QList>
#include <QPointer>

#include <ui/ObjectDrawStruct.h>

namespace panda
{

class DockableObject;

class DockObject : public PandaObject
{
public:
	DockObject(QObject *parent);
	virtual ~DockObject();

	virtual bool accepts(DockableObject* dockable) const;
	virtual void addDockable(DockableObject* dockable, int index = -1);
	virtual void removeDockable(DockableObject* dockable);

	typedef QListIterator<DockableObject*> DockablesIterator;
	virtual DockablesIterator getDockablesIterator() const;

	virtual int getIndexOfDockable(DockableObject* dockable) const;

protected:
	virtual void doRemoveInput(DataNode* node);

	QList<DockableObject*> dockedObjects;
};

class DockableObject : public PandaObject
{
public:
	DockableObject(QObject *parent);
	virtual void setParentDock(DockObject* dock);
	virtual DockObject* getParentDock();
	virtual DockObject* getDefaultDock(PandaDocument* doc);

protected:
	QPointer<DockObject> parentDock;
};

} // namespace panda

class DockObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockObjectDrawStruct(GraphView* view, panda::DockObject* object);

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
	DockableObjectDrawStruct(GraphView* view, panda::DockableObject* object);

	virtual void drawShape(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);

	static const int dockableCircleWidth = 20;
protected:
	QPainterPath shapePath;
};

#endif
