#ifndef GROUPOBJECTDRAWSTRUCT_H
#define GROUPOBJECTDRAWSTRUCT_H

#include <panda/GenericObject.h>

#include <ui/ObjectDrawStruct.h>

class GraphView;

class GroupObjectDrawStruct : public ObjectDrawStruct
{
public:
	GroupObjectDrawStruct(GraphView* view, panda::PandaObject* object);

	virtual void drawShape(QPainter* painter);
	virtual void drawText(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);
	virtual int dataStartY();

protected:
	QPainterPath shapePath;
};

#endif
