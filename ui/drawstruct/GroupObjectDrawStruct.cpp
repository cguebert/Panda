#include <ui/drawstruct/GroupObjectDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/Group.h>

#include <QPainter>

GroupObjectDrawStruct::GroupObjectDrawStruct(GraphView* view, panda::Group* object)
	: ObjectDrawStruct(view, object)
	, group(object)
{
	update();
}

void GroupObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(shapePath);
}

void GroupObjectDrawStruct::drawText(QPainter* painter)
{
	if(group && !group->groupName.getValue().isEmpty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = objectArea.adjusted(margin, 0, -margin, 0);
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, group->groupName.getValue());
	}
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
