#ifndef DOCKABLEDRAWSTRUCT_H
#define DOCKABLEDRAWSTRUCT_H

#include <panda/object/Dockable.h>

#include <ui/drawstruct/ObjectDrawStruct.h>

class DockObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockObjectDrawStruct(GraphView* view, panda::DockObject* m_object);

	void placeDockableObjects();

	void drawShape(QPainter* painter) override;
	QSize getObjectSize() override;
	QRectF getTextArea() override;
	void move(const QPointF& delta) override;
	void moveVisual(const QPointF& delta) override;
	void update() override;
	bool contains(const QPointF& point) override;

	int getDockableIndex(const QRectF& rect);

	static const int dockEmptyRendererHeight = 30;
	static const int dockHoleWidth = 80;
	static const int dockHoleMargin = 3;
	static const int dockRendererMargin = 10;
protected:
	panda::DockObject* m_dockObject;
	QPainterPath m_shapePath;
	QList<int> m_dockablesY;	// Used only in "getDockableIndex"
};

class DockableObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockableObjectDrawStruct(GraphView* view, panda::DockableObject* m_object);

	void drawShape(QPainter* painter) override;
	void moveVisual(const QPointF& delta) override;
	void update() override;
	bool contains(const QPointF& point) override;
	QSize getObjectSize() override;
	QRectF getTextArea() override;

	static const int dockableCircleWidth = 20;

	static const int dockableWithOutputAdds = 25;
	static const int dockableWithOutputRect = 15; // How much rectangle area we add
	static const int dockableWithOutputArc = 15;

protected:
	QPainterPath m_shapePath;
	bool m_hasOutputs = false;
};

#endif
