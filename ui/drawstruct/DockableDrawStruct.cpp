#include <panda/PandaDocument.h>

#include <ui/GraphView.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/command/MoveObjectCommand.h>

#include <QPainter>

DockObjectDrawStruct::DockObjectDrawStruct(GraphView* view, panda::DockObject* object)
	: ObjectDrawStruct(view, object)
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
	painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, QString::fromStdString(m_object->getName()));
}

QSize DockObjectDrawStruct::getObjectSize()
{
	QSize temp = ObjectDrawStruct::getObjectSize();
	temp.rwidth() += 20;
	temp.rheight() += dockEmptyRendererHeight + dockRendererMargin * 2;

	for(auto dockable : m_dockObject->getDockedObjects())
		temp.rheight() += m_parentView->getObjectDrawStruct(dockable)->getObjectSize().height() + dockRendererMargin;

	return temp;
}

void DockObjectDrawStruct::move(const QPointF& delta)
{
	ObjectDrawStruct::move(delta);
	for(auto dockable : m_dockObject->getDockedObjects())
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

	for(auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectDrawStruct* objectStruct = m_parentView->getObjectDrawStruct(dockable);
		QSize objectSize = objectStruct->getObjectSize();
		QPointF objectNewPos(m_position.x() + dockHoleWidth - objectSize.width(), m_position.y() + ty - m_objectArea.top());

		auto doc = m_parentView->getDocument();
		if(doc->isInCommandMacro())
			doc->addCommand(std::make_shared<MoveObjectCommand>(m_parentView, dockable, objectNewPos - objectStruct->getPosition()));

		m_dockablesY.push_back(objectStruct->getObjectArea().top());

		tx = m_objectArea.left() + dockHoleWidth - DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int w = DockableObjectDrawStruct::dockableCircleWidth;
		int h = objectSize.height();

		path.lineTo(m_objectArea.left(), ty - dockHoleMargin);
		path.arcTo(tx - w - dockHoleMargin, ty - dockHoleMargin, w * 2 + dockHoleMargin, h + dockHoleMargin * 2, 90, -180);
		path.lineTo(m_objectArea.left(), ty + h + dockHoleMargin);

		ty += objectSize.height() + dockRendererMargin;
	}

	ty = m_objectArea.bottom()-dockEmptyRendererHeight-dockRendererMargin;
	path.lineTo(m_objectArea.left(), ty);
	tx = m_objectArea.left()+dockHoleWidth-DockableObjectDrawStruct::dockableCircleWidth;
	path.arcTo(tx, ty, DockableObjectDrawStruct::dockableCircleWidth, dockEmptyRendererHeight, 90, -180);
	path.lineTo(m_objectArea.left(), ty+dockEmptyRendererHeight);
	path.closeSubpath();
	path.swap(m_shapePath);
}

int DockObjectDrawStruct::getDockableIndex(const QRectF& rect)
{
	int y = rect.top();
	for(int i=0; i<m_dockablesY.size(); ++i)
	{
		if(y < m_dockablesY[i])
			return i;
	}
	return -1;
}

int dockObjectDrawClass = RegisterDrawObject<panda::DockObject, DockObjectDrawStruct>();

//****************************************************************************//

DockableObjectDrawStruct::DockableObjectDrawStruct(GraphView* view, panda::DockableObject* dockable)
	: ObjectDrawStruct(view, dockable)
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

QSize DockableObjectDrawStruct::getObjectSize()
{
	auto size = ObjectDrawStruct::getObjectSize();
	if (!getObject()->getOutputDatas().empty())
		size.rwidth() += dockableWithOutputAdds;
	return size;
}

void DockableObjectDrawStruct::update()
{
	ObjectDrawStruct::update();

	QPainterPath path;
	path.moveTo(m_objectArea.left(), m_objectArea.center().y());
	path.arcTo(m_objectArea.left(), m_objectArea.top(), 10, 10, 180, -90);
	path.arcTo(m_objectArea.right() - dockableCircleWidth * 2, m_objectArea.top(), dockableCircleWidth * 2, m_objectArea.height(), 90, -180);
	path.arcTo(m_objectArea.left(), m_objectArea.bottom() - 10, 10, 10, 270, -90);
	path.closeSubpath();
	path.swap(m_shapePath);
}

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectDrawStruct>();

