#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <ui/GraphView.h>
#include <ui/GroupObjectDrawStruct.h>

#include <QPainter>

using panda::GenericObject;

GroupObjectDrawStruct::GroupObjectDrawStruct(GraphView* view, panda::PandaObject* object)
	: ObjectDrawStruct(view, (panda::PandaObject*)object)
{
	update();
}

void GroupObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(shapePath);
}

void GroupObjectDrawStruct::drawText(QPainter* painter)
{
	panda::Group* group = dynamic_cast<panda::Group*>(this->object);
	if(group && group->groupName.getValue().size())
		painter->drawText(objectArea, Qt::AlignCenter, group->groupName.getValue());
	else
		ObjectDrawStruct::drawText(painter);
}

void GroupObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	shapePath.translate(delta);
}

bool GroupObjectDrawStruct::contains(const QPointF& point)
{
	return shapePath.contains(point);
}

void GroupObjectDrawStruct::update()
{
	const int w = 9, h = 9;
	ObjectDrawStruct::update();

	QPainterPath path;
	path.moveTo(objectArea.left()+w, objectArea.top());
	path.lineTo(objectArea.right()-w, objectArea.top());
	path.lineTo(objectArea.right(), objectArea.top()+h);
	path.lineTo(objectArea.right(), objectArea.bottom()-h);
	path.lineTo(objectArea.right()-w, objectArea.bottom());
	path.lineTo(objectArea.left()+w, objectArea.bottom());
	path.lineTo(objectArea.left(), objectArea.bottom()-h);
	path.lineTo(objectArea.left(), objectArea.top()+h);
	path.lineTo(objectArea.left()+w, objectArea.top());
	path.swap(shapePath);
}

int GroupObjectDrawStruct::dataStartY()
{
	return 10;
}

int GroupObjectDrawClass = RegisterDrawObject<panda::Group, GroupObjectDrawStruct>();
