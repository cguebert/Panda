#ifndef GROUPOBJECTDRAWSTRUCT_H
#define GROUPOBJECTDRAWSTRUCT_H

#include <ui/drawstruct/ObjectDrawStruct.h>

namespace panda
{
class Group;
}

class GraphView;

class GroupObjectDrawStruct : public ObjectDrawStruct
{
public:
	GroupObjectDrawStruct(GraphView* view, panda::Group* m_object);

	virtual void drawShape(QPainter* painter);
	virtual void drawText(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);
	virtual int dataStartY();

protected:
	QPainterPath shapePath;
	panda::Group* group;
};

#endif
