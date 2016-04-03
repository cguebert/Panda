#include <panda/PandaDocument.h>

#include <ui/graphview/GraphView.h>
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

QSize DockObjectDrawStruct::getObjectSize()
{
	QSize temp = ObjectDrawStruct::getObjectSize();
	temp.rwidth() += 20;
	temp.rheight() += dockEmptyRendererHeight + dockRendererMargin * 2;

	for(auto dockable : m_dockObject->getDockedObjects())
		temp.rheight() += m_parentView->getObjectDrawStruct(dockable)->getObjectSize().height() + dockRendererMargin;

	return temp;
}

QRectF DockObjectDrawStruct::getTextArea()
{
	int margin = dataRectSize+dataRectMargin+3;
	QRectF textArea = m_objectArea;
	textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
	textArea.adjust(margin, 0, -margin, 0);
	return textArea;
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

	const int cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int dhm = dockHoleMargin;
	const int rw = DockableObjectDrawStruct::dockableWithOutputRect;
	const int aw = DockableObjectDrawStruct::dockableWithOutputArc;
	const int ah = aw - dockHoleMargin * 2;

	int tx, ty;
	ty = m_objectArea.top() + ObjectDrawStruct::getObjectSize().height() + dockRendererMargin;

	for(auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectDrawStruct* objectStruct = m_parentView->getObjectDrawStruct(dockable);
		QSize objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();
		QPointF objectNewPos(m_position.x() + dockHoleWidth - objectSize.width(), m_position.y() + ty - m_objectArea.top());
		
		// If the object has outputs, it is drawn larger but must be placed at the same position
		if (hasOutputs)
			objectNewPos.rx() += DockableObjectDrawStruct::dockableWithOutputAdds;

		auto doc = m_parentView->getDocument();
		if(doc->getUndoStack().isInCommandMacro())
			doc->getUndoStack().push(std::make_shared<MoveObjectCommand>(m_parentView, dockable, objectNewPos - objectStruct->getPosition()));

		QRectF objectArea = objectStruct->getObjectArea();
		m_dockablesY.push_back(objectArea.top());

		tx = m_objectArea.left() + dockHoleWidth - DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int w = DockableObjectDrawStruct::dockableCircleWidth;
		int h = objectSize.height();

		path.lineTo(m_objectArea.left(), ty - dockHoleMargin);
		if (hasOutputs)
		{
			const int top = objectArea.top() - dhm, bot = objectArea.bottom() + dhm;

			// Arc at the top
			path.arcTo(objectArea.right() - rw - aw * 3, top, aw * 2, ah * 2, 90, -90);
			path.arcTo(objectArea.right() - rw - aw, top, aw * 2, ah * 2, -180, -90);

			path.arcTo(objectArea.right() - cr + dhm, top, cr, cr, 90, -90); // Top right corner
			path.arcTo(objectArea.right() - cr + dhm, bot - cr, cr, cr, 0, -90); // Bottom right corner

			// Arc at the bottom
			path.arcTo(objectArea.right() - rw - aw, bot - ah * 2, aw * 2, ah * 2, -90, -90);
			path.arcTo(objectArea.right() - rw - aw * 3, bot - ah * 2, aw * 2, ah * 2, 0, -90);
		}
		else
			path.arcTo(tx - w - dockHoleMargin, ty - dockHoleMargin, w * 2 + dockHoleMargin, h + dockHoleMargin * 2, 90, -180);
		path.lineTo(m_objectArea.left(), ty + h + dockHoleMargin);

		ty += h + dockRendererMargin;
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
	if (m_hasOutputs)
		size.rwidth() += dockableWithOutputAdds;
	return size;
}

QRectF DockableObjectDrawStruct::getTextArea()
{
	auto area = ObjectDrawStruct::getTextArea();
	if (m_hasOutputs)
		area.adjust(0, 0, -dockableWithOutputAdds, 0);
	return area;
}

void DockableObjectDrawStruct::update()
{
	m_hasOutputs = !getObject()->getOutputDatas().empty();
	ObjectDrawStruct::update();

	const int cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int rw = dockableWithOutputRect;
	const int aw = dockableWithOutputArc;

	if (m_hasOutputs)
	{
		QPainterPath path;
		path.moveTo(m_objectArea.left(), m_objectArea.center().y());
		path.arcTo(m_objectArea.left(), m_objectArea.top(), cr, cr, 180, -90); // Top left corner

		// Arc at the top
		path.arcTo(m_objectArea.right() - rw - aw * 3, m_objectArea.top(), aw * 2, aw * 2, 90, -90);
		path.arcTo(m_objectArea.right() - rw - aw, m_objectArea.top(), aw * 2, aw * 2, -180, -90);

		path.arcTo(m_objectArea.right() - cr, m_objectArea.top(), cr, cr, 90, -90); // Top right corner
		path.arcTo(m_objectArea.right() - cr, m_objectArea.bottom() - cr, cr, cr, 0, -90); // Bottom right corner

		// Arc at the bottom
		path.arcTo(m_objectArea.right() - rw - aw, m_objectArea.bottom() - aw * 2, aw * 2, aw * 2, -90, -90);
		path.arcTo(m_objectArea.right() - rw - aw * 3, m_objectArea.bottom() - aw * 2, aw * 2, aw * 2, 0, -90);

		path.arcTo(m_objectArea.left(), m_objectArea.bottom() - cr, cr, cr, 270, -90); // Bottom left corner
		path.closeSubpath();
		path.swap(m_shapePath);
	}
	else
	{
		QPainterPath path;
		path.moveTo(m_objectArea.left(), m_objectArea.center().y());
		path.arcTo(m_objectArea.left(), m_objectArea.top(), cr, cr, 180, -90); // Top left corner
		path.arcTo(m_objectArea.right() - dockableCircleWidth * 2, m_objectArea.top(), 
			dockableCircleWidth * 2, m_objectArea.height(), 90, -180); // Right side arc
		path.arcTo(m_objectArea.left(), m_objectArea.bottom() - cr, cr, cr, 270, -90); // Bottom left corner
		path.closeSubpath();
		path.swap(m_shapePath);
	}
}

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectDrawStruct>();

