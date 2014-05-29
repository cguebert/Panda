#include <panda/PandaDocument.h>

#include <ui/GraphView.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/command/MoveObjectCommand.h>

#include <QPainter>

DockObjectDrawStruct::DockObjectDrawStruct(GraphView* view, panda::DockObject* object)
	: ObjectDrawStruct(view, (panda::PandaObject*)object)
	, m_dockObject(object)
{
	update();
}

void DockObjectDrawStruct::drawShape(QPainter* painter)
{
	painter->drawPath(m_shapePath);
}

void DockObjectDrawStruct::drawText(QPainter* painter)
{
	int margin = dataRectSize+dataRectMargin+3;
	QRectF textArea = m_objectArea;
	textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
	textArea.adjust(margin, 0, -margin, 0);
	painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, m_object->getName());
}

QSize DockObjectDrawStruct::getObjectSize()
{
	QSize temp = ObjectDrawStruct::getObjectSize();
	temp.rwidth() += 20;
	temp.rheight() += dockEmptyRendererHeight + dockRendererMargin * 2;

	for(auto dockable : m_dockObject->getDockables())
		temp.rheight() += m_parentView->getObjectDrawStruct(dockable)->getObjectSize().height() + dockRendererMargin;

	return temp;
}

void DockObjectDrawStruct::move(const QPointF& delta)
{
	ObjectDrawStruct::move(delta);
	for(auto dockable : m_dockObject->getDockables())
		m_parentView->getObjectDrawStruct(dockable)->move(delta);
}

void DockObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	m_shapePath.translate(delta);
}

bool DockObjectDrawStruct::contains(const QPointF& point)
{
	return m_shapePath.contains(point);
}

void DockObjectDrawStruct::update()
{
	ObjectDrawStruct::update();

	m_dockablesY.clear();

	QPainterPath path;
	path.moveTo(m_objectArea.left(), m_objectArea.bottom());
	path.lineTo(m_objectArea.right(), m_objectArea.bottom());
	path.lineTo(m_objectArea.right(), m_objectArea.top());
	path.lineTo(m_objectArea.left(), m_objectArea.top());

	int tx, ty;
	ty = m_objectArea.top() + ObjectDrawStruct::getObjectSize().height() + dockRendererMargin;

	for(auto dockable : m_dockObject->getDockables())
	{
		ObjectDrawStruct* objectStruct = m_parentView->getObjectDrawStruct(dockable);
		QSize objectSize = objectStruct->getObjectSize();
		QPointF objectNewPos(m_position.x() + dockHoleWidth - objectSize.width(), m_position.y() + ty - m_objectArea.top());

		auto doc = m_parentView->getDocument();
		if(doc->isInCommandMacro())
			doc->addCommand(new MoveObjectCommand(m_parentView, dockable, objectNewPos - objectStruct->getPosition()));

		m_dockablesY.push_back(objectStruct->getObjectArea().center().y());

		tx = m_objectArea.left() + dockHoleWidth - DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int w = DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int h = objectSize.height() + dockHoleMargin * 2 - 1;

		path.lineTo(m_objectArea.left(), ty - dockHoleMargin - 1);
		path.lineTo(tx, ty - dockHoleMargin - 1);
		path.cubicTo(tx+w/2.0, ty-dockHoleMargin - 1,
					 tx+w, ty+h/4.0,
					 tx+w, ty+h/2.0);
		path.cubicTo(tx+w, ty+h*3.0/4.0,
					 tx+w/2.0, ty+h,
					 tx, ty+h);
		path.lineTo(m_objectArea.left(), ty+h);

		ty += objectSize.height() + dockRendererMargin;
	}

	ty = m_objectArea.bottom()-dockEmptyRendererHeight-dockRendererMargin;
	path.lineTo(m_objectArea.left(), ty);
	tx = m_objectArea.left()+dockHoleWidth-DockableObjectDrawStruct::dockableCircleWidth;
	path.lineTo(tx, ty);
	path.cubicTo(tx+DockableObjectDrawStruct::dockableCircleWidth/2.0, ty,
				 tx+DockableObjectDrawStruct::dockableCircleWidth, ty+dockEmptyRendererHeight/4.0,
				 tx+DockableObjectDrawStruct::dockableCircleWidth, ty+dockEmptyRendererHeight/2.0);
	path.cubicTo(tx+DockableObjectDrawStruct::dockableCircleWidth, ty+dockEmptyRendererHeight*3.0/4.0,
				 tx+DockableObjectDrawStruct::dockableCircleWidth/2.0, ty+dockEmptyRendererHeight,
				 tx, ty+dockEmptyRendererHeight);
	path.lineTo(m_objectArea.left(), ty+dockEmptyRendererHeight);
	path.lineTo(m_objectArea.left(), m_objectArea.bottom());
	path.swap(m_shapePath);
}

int DockObjectDrawStruct::getDockableIndex(const QRectF& rect)
{
	int y = rect.center().y();
	for(int i=0; i<m_dockablesY.size(); ++i)
	{
		if(y < m_dockablesY[i])
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
	painter->drawPath(m_shapePath);
}

void DockableObjectDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	m_shapePath.translate(delta);
}

bool DockableObjectDrawStruct::contains(const QPointF& point)
{
	return m_shapePath.contains(point);
}

void DockableObjectDrawStruct::update()
{
	ObjectDrawStruct::update();

	QPainterPath path;
	path.moveTo(m_objectArea.left(), m_objectArea.center().y());
	path.lineTo(m_objectArea.left(), m_objectArea.top()+5);
	path.cubicTo(m_objectArea.left(), m_objectArea.top()+2.5,
				 m_objectArea.left()+2.5, m_objectArea.top(),
				 m_objectArea.left()+5, m_objectArea.top());
	path.lineTo(m_objectArea.right()-dockableCircleWidth, m_objectArea.top());
	path.cubicTo(m_objectArea.right()-dockableCircleWidth/2.0, m_objectArea.top(),
				 m_objectArea.right(), m_objectArea.top()+m_objectArea.height()/4.0,
				 m_objectArea.right(), m_objectArea.center().y());
	path.cubicTo(m_objectArea.right(), m_objectArea.bottom()-m_objectArea.height()/4.0,
				 m_objectArea.right()-dockableCircleWidth/2.0, m_objectArea.bottom(),
				 m_objectArea.right()-dockableCircleWidth, m_objectArea.bottom());
	path.lineTo(m_objectArea.left()+5, m_objectArea.bottom());
	path.cubicTo(m_objectArea.left()+2.5, m_objectArea.bottom(),
				 m_objectArea.left(), m_objectArea.bottom()-2.5,
				 m_objectArea.left(), m_objectArea.bottom()-5);
	path.lineTo(m_objectArea.left(), m_objectArea.center().y());
	path.swap(m_shapePath);
}

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectDrawStruct>();

