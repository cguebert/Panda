#ifndef GROUPOBJECTDRAWSTRUCT_H
#define GROUPOBJECTDRAWSTRUCT_H

#include <ui/drawstruct/ObjectDrawStruct.h>

namespace panda
{
class Group;
}

class GraphView;

class GroupDrawStruct : public ObjectDrawStruct
{
public:
	GroupDrawStruct(GraphView* view, panda::Group* object);

	virtual void drawShape(QPainter* painter);
	virtual void drawText(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);
	virtual int dataStartY();

protected:
	QPainterPath m_shapePath;
	panda::Group* m_group;
};

#endif
