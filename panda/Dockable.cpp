#include <panda/Dockable.h>
#include <panda/PandaDocument.h>

#include <ui/GraphView.h>

#include <QPainter>

namespace panda
{

DockObject::DockObject(QObject *parent)
	: PandaObject(parent)
{
}

DockObject::~DockObject()
{
	DockObject* defaultDock = NULL;
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());

	DockablesIterator iter(dockedObjects);
	while(iter.hasNext())
	{
		DockableObject* dockable = iter.next();
		removeInput((DataNode*)dockable);

		if(doc)
			defaultDock = dockable->getDefaultDock(doc);

		if(defaultDock == this)
			defaultDock = NULL;

		dockable->setParentDock(defaultDock);

		if(defaultDock)
			defaultDock->addDockable(dockable);
	}
}

bool DockObject::accepts(DockableObject* /*dockable*/) const
{
	return true;
}

void DockObject::addDockable(DockableObject* dockable, int index)
{
	dockable->setParentDock(this);
	addInput((DataNode*)dockable);
	if(index < 0)
		dockedObjects.append(dockable);
	else
		dockedObjects.insert(index, dockable);
	emit modified(this);
}

void DockObject::removeDockable(DockableObject* dockable)
{
	removeInput((DataNode*)dockable);
}

DockObject::DockablesIterator DockObject::getDockablesIterator() const
{
	return DockablesIterator(dockedObjects);
}

int DockObject::getIndexOfDockable(DockableObject* dockable) const
{
	return dockedObjects.indexOf(dockable);
}

void DockObject::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);

	if(dockedObjects.contains((DockableObject*)node))
	{
		dockedObjects.removeAll((DockableObject*)node);
		setDirtyValue();
		emit modified(this);
	}
}

//******************************************************************************************

DockableObject::DockableObject(QObject *parent)
	: PandaObject(parent)
{
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent);
	if(doc)
	{
		DockObject* dock = getDefaultDock(doc);
		if(dock)
			dock->addDockable(this);
	}
}

void DockableObject::setParentDock(DockObject* dock)
{
	parentDock = dock;
}

DockObject* DockableObject::getParentDock()
{
	return parentDock.data();
}

DockObject* DockableObject::getDefaultDock(PandaDocument* /*doc*/)
{
	return NULL;
}

} // namespace panda

//******************************************************************************************

DockObjectDrawStruct::DockObjectDrawStruct(GraphView* view, panda::DockObject* object)
	: ObjectDrawStruct(view, (panda::PandaObject*)object)
	, dockObject(object)
{
	update();
}

void DockObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(shapePath);
}

void DockObjectDrawStruct::drawText(QPainter* painter)
{
	QRectF textArea = objectArea;
	textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
	painter->drawText(textArea, Qt::AlignCenter, object->getName());
}

QSize DockObjectDrawStruct::getObjectSize()
{
	QSize temp = ObjectDrawStruct::getObjectSize();
	temp.rwidth() += 20;
	temp.rheight() += dockEmptyRendererHeight + dockRendererMargin * 2;

	panda::DockObject::DockablesIterator iter = dockObject->getDockablesIterator();
	while(iter.hasNext())
	{
		panda::DockableObject* dockable = iter.next();
		temp.rheight() += parentView->getObjectDrawStruct(dockable)->getObjectSize().height() + dockRendererMargin;
	}

	return temp;
}

void DockObjectDrawStruct::move(const QPointF& delta)
{
	ObjectDrawStruct::move(delta);
	panda::DockObject::DockablesIterator iter = dockObject->getDockablesIterator();
	while(iter.hasNext())
		parentView->getObjectDrawStruct(iter.next())->move(delta);
}

void DockObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	shapePath.translate(delta);
}

bool DockObjectDrawStruct::contains(const QPointF& point)
{
	return shapePath.contains(point);
}

void DockObjectDrawStruct::update()
{
	ObjectDrawStruct::update();

	dockablesY.clear();

	QPainterPath path;
	path.moveTo(objectArea.left(), objectArea.bottom());
	path.lineTo(objectArea.right(), objectArea.bottom());
	path.lineTo(objectArea.right(), objectArea.top());
	path.lineTo(objectArea.left(), objectArea.top());

	int tx, ty;
	ty = objectArea.top() + ObjectDrawStruct::getObjectSize().height() + dockRendererMargin;

	panda::DockObject::DockablesIterator iter = dockObject->getDockablesIterator();
	while(iter.hasNext())
	{
		panda::DockableObject* dockable = iter.next();
		ObjectDrawStruct* objectStruct = parentView->getObjectDrawStruct(dockable);
		QSize objectSize = objectStruct->getObjectSize();
		QPointF objectNewPos(position.x() + dockHoleWidth - objectSize.width(), position.y() + ty - objectArea.top());
		objectStruct->move(objectNewPos - objectStruct->getPosition());

		dockablesY.append(objectStruct->getObjectArea().center().y());

		tx = objectArea.left() + dockHoleWidth - DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int w = DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int h = objectSize.height() + dockHoleMargin * 2 - 1;

		path.lineTo(objectArea.left(), ty - dockHoleMargin - 1);
		path.lineTo(tx, ty - dockHoleMargin - 1);
		path.cubicTo(tx+w/2.0, ty-dockHoleMargin - 1,
					 tx+w, ty+h/4.0,
					 tx+w, ty+h/2.0);
		path.cubicTo(tx+w, ty+h*3.0/4.0,
					 tx+w/2.0, ty+h,
					 tx, ty+h);
		path.lineTo(objectArea.left(), ty+h);

		ty += objectSize.height() + dockRendererMargin;
	}

	ty = objectArea.bottom()-dockEmptyRendererHeight-dockRendererMargin;
	path.lineTo(objectArea.left(), ty);
	tx = objectArea.left()+dockHoleWidth-DockableObjectDrawStruct::dockableCircleWidth;
	path.lineTo(tx, ty);
	path.cubicTo(tx+DockableObjectDrawStruct::dockableCircleWidth/2.0, ty,
				 tx+DockableObjectDrawStruct::dockableCircleWidth, ty+dockEmptyRendererHeight/4.0,
				 tx+DockableObjectDrawStruct::dockableCircleWidth, ty+dockEmptyRendererHeight/2.0);
	path.cubicTo(tx+DockableObjectDrawStruct::dockableCircleWidth, ty+dockEmptyRendererHeight*3.0/4.0,
				 tx+DockableObjectDrawStruct::dockableCircleWidth/2.0, ty+dockEmptyRendererHeight,
				 tx, ty+dockEmptyRendererHeight);
	path.lineTo(objectArea.left(), ty+dockEmptyRendererHeight);
	path.lineTo(objectArea.left(), objectArea.bottom());
	path.swap(shapePath);
}

int DockObjectDrawStruct::getDockableIndex(const QRectF& rect)
{
	int y = rect.center().y();
	for(int i=0; i<dockablesY.size(); ++i)
	{
		if(y < dockablesY[i])
			return i;
	}
	return -1;
}

int dockObjectDrawClass = RegisterDrawObject<panda::DockObject, DockObjectDrawStruct>();

//******************************************************************************************

DockableObjectDrawStruct::DockableObjectDrawStruct(GraphView* view, panda::DockableObject* dockable)
	: ObjectDrawStruct(view, (panda::PandaObject*)dockable)
{
	update();
}

void DockableObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(shapePath);
}

void DockableObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	shapePath.translate(delta);
}

bool DockableObjectDrawStruct::contains(const QPointF& point)
{
	return shapePath.contains(point);
}

void DockableObjectDrawStruct::update()
{
	ObjectDrawStruct::update();

	QPainterPath path;
	path.moveTo(objectArea.left(), objectArea.center().y());
	path.lineTo(objectArea.left(), objectArea.top()+5);
	path.cubicTo(objectArea.left(), objectArea.top()+2.5,
				 objectArea.left()+2.5, objectArea.top(),
				 objectArea.left()+5, objectArea.top());
	path.lineTo(objectArea.right()-dockableCircleWidth, objectArea.top());
	path.cubicTo(objectArea.right()-dockableCircleWidth/2.0, objectArea.top(),
				 objectArea.right(), objectArea.top()+objectArea.height()/4.0,
				 objectArea.right(), objectArea.center().y());
	path.cubicTo(objectArea.right(), objectArea.bottom()-objectArea.height()/4.0,
				 objectArea.right()-dockableCircleWidth/2.0, objectArea.bottom(),
				 objectArea.right()-dockableCircleWidth, objectArea.bottom());
	path.lineTo(objectArea.left()+5, objectArea.bottom());
	path.cubicTo(objectArea.left()+2.5, objectArea.bottom(),
				 objectArea.left(), objectArea.bottom()-2.5,
				 objectArea.left(), objectArea.bottom()-5);
	path.lineTo(objectArea.left(), objectArea.center().y());
	path.swap(shapePath);
}

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectDrawStruct>();

