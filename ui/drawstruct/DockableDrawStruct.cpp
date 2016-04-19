#include <panda/PandaDocument.h>

#include <ui/graphview/GraphView.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/command/MoveObjectCommand.h>

using panda::types::Point;
using panda::types::Rect;

DockObjectDrawStruct::DockObjectDrawStruct(GraphView* view, panda::DockObject* object)
	: ObjectDrawStruct(view, object)
	, m_dockObject(object)
{
	update();
}

void DockObjectDrawStruct::drawShape(DrawList& list, DrawColors& colors)
{
	list.addMesh(m_shapeMesh, colors.fillColor);
	list.addPolyline(m_shapePath, colors.penColor, false, colors.penWidth);
}

Point DockObjectDrawStruct::getObjectSize()
{
	Point temp = ObjectDrawStruct::getObjectSize();
	temp.x += 20;
	temp.y += dockEmptyRendererHeight + dockRendererMargin * 2;

	for(auto dockable : m_dockObject->getDockedObjects())
		temp.y += m_parentView->getObjectDrawStruct(dockable)->getObjectSize().y + dockRendererMargin;

	return temp;
}

Rect DockObjectDrawStruct::getTextArea()
{
	int margin = dataRectSize+dataRectMargin+3;
	Rect textArea = m_objectArea;
	textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
	textArea.adjust(margin, 0, -margin, 0);
	return textArea;
}

void DockObjectDrawStruct::move(const Point& delta)
{
	ObjectDrawStruct::move(delta);
	for(auto dockable : m_dockObject->getDockedObjects())
		m_parentView->getObjectDrawStruct(dockable)->move(delta);
}

bool DockObjectDrawStruct::contains(const Point& point)
{
	return m_shapePath.contains(point);
}

void DockObjectDrawStruct::placeDockableObjects()
{
	m_dockablesY.clear();

	const int cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int dhm = dockHoleMargin;
	const int rw = DockableObjectDrawStruct::dockableWithOutputRect;
	const int aw = DockableObjectDrawStruct::dockableWithOutputArc;
	const int ah = aw - dockHoleMargin * 2;

	int ty;
	ty = m_objectArea.top() + ObjectDrawStruct::getObjectSize().y + dockRendererMargin;

	auto doc = m_parentView->getDocument();
	auto& undoStack = doc->getUndoStack();
	bool canMoveObjects = doc->getUndoStack().isInCommandMacro();

	for (auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectDrawStruct* objectStruct = m_parentView->getObjectDrawStruct(dockable);
		Point objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();
		Point objectNewPos(m_position.x + dockHoleWidth - objectSize.x, m_position.y + ty - m_objectArea.top());

		// If the object has outputs, it is drawn larger but must be placed at the same position
		if (hasOutputs)
			objectNewPos.x += DockableObjectDrawStruct::dockableWithOutputAdds;

		if (canMoveObjects && objectNewPos != objectStruct->getPosition())
			undoStack.push(std::make_shared<MoveObjectCommand>(m_parentView, dockable, objectNewPos - objectStruct->getPosition()));

		Rect objectArea = objectStruct->getObjectArea();
		m_dockablesY.push_back(objectArea.top());

		ty += objectSize.y + dockRendererMargin;
	}
}

void DockObjectDrawStruct::update()
{
	ObjectDrawStruct::update();

	m_shapePath.clear();
	m_shapePath.moveTo(m_objectArea.bottomLeft());
	m_shapePath.lineTo(m_objectArea.bottomRight());
	m_shapePath.lineTo(m_objectArea.topRight());
	m_shapePath.lineTo(m_objectArea.topLeft());

	const int cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int dhm = dockHoleMargin;
	const int rw = DockableObjectDrawStruct::dockableWithOutputRect;
	const int aw = DockableObjectDrawStruct::dockableWithOutputArc;
	const int ah = aw - dockHoleMargin * 2;

	int tx, ty;
	ty = m_objectArea.top() + ObjectDrawStruct::getObjectSize().y + dockRendererMargin;

	for(auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectDrawStruct* objectStruct = m_parentView->getObjectDrawStruct(dockable);
		Point objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();

		tx = m_objectArea.left() + dockHoleWidth - DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int w = DockableObjectDrawStruct::dockableCircleWidth;
		int h = objectSize.y;

		Rect objectArea = objectStruct->getObjectArea();
		m_shapePath.lineTo(Point(m_objectArea.left(), ty - dockHoleMargin));
		if (hasOutputs)
		{
			const int top = objectArea.top() - dhm, bot = objectArea.bottom() + dhm;
			const auto right = objectArea.right();

			// Arc at the top
			m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw * 3, top, aw * 2, ah * 2), -90, 90);
			m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw, top, aw * 2, ah * 2), 180, 90);

			m_shapePath.arcToDegrees(Rect::fromSize(right - cr + dhm, top, cr, cr), -90, 90); // Top right corner
			m_shapePath.arcToDegrees(Rect::fromSize(right - cr + dhm, bot - cr, cr, cr), 0, 90); // Bottom right corner

			// Arc at the bottom
			m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw, bot - ah * 2, aw * 2, ah * 2), 90, 90);
			m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw * 3, bot - ah * 2, aw * 2, ah * 2), 0, 90);
		}
		else
			m_shapePath.arcToDegrees(Rect::fromSize(tx - w - dockHoleMargin, ty - dockHoleMargin, w * 2 + dockHoleMargin, h + dockHoleMargin * 2), -90, 180);
		m_shapePath.lineTo(Point(m_objectArea.left(), ty + h + dockHoleMargin));

		ty += h + dockRendererMargin;
	}

	ty = m_objectArea.bottom()-dockEmptyRendererHeight-dockRendererMargin;
	m_shapePath.lineTo(Point(m_objectArea.left(), ty));
	tx = m_objectArea.left()+dockHoleWidth-DockableObjectDrawStruct::dockableCircleWidth;
	m_shapePath.arcToDegrees(Rect::fromSize(tx, ty, DockableObjectDrawStruct::dockableCircleWidth, dockEmptyRendererHeight), -90, 180);
	m_shapePath.lineTo(Point(m_objectArea.left(), ty+dockEmptyRendererHeight));
	m_shapePath.close();

	m_shapeMesh = m_shapePath.triangulate();
}

int DockObjectDrawStruct::getDockableIndex(const Rect& rect)
{
	int y = rect.top();
	int nb = m_dockablesY.size();
	for (int i = 0; i < nb; ++i)
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

void DockableObjectDrawStruct::drawShape(DrawList& list, DrawColors& colors)
{
	list.addMesh(m_shapeMesh, colors.fillColor);
	list.addPolyline(m_shapePath, colors.penColor, false, colors.penWidth);
}

bool DockableObjectDrawStruct::contains(const Point& point)
{
	return m_shapePath.contains(point);
}

Point DockableObjectDrawStruct::getObjectSize()
{
	auto size = ObjectDrawStruct::getObjectSize();
	if (m_hasOutputs)
		size.x += dockableWithOutputAdds;
	return size;
}

Rect DockableObjectDrawStruct::getTextArea()
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
	const auto left = m_objectArea.left(), top = m_objectArea.top(), 
		right = m_objectArea.right(), bottom = m_objectArea.bottom();

	if (m_hasOutputs)
	{
		m_shapePath.clear();
		m_shapePath.moveTo(Point(left, m_objectArea.center().y));
		m_shapePath.arcToDegrees(Rect::fromSize(left, top, cr, cr), 180, 90); // Top left corner

		// Arc at the top
		m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw * 3, top, aw * 2, aw * 2), -90, 90);
		m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw, top, aw * 2, aw * 2), 180, 90);

		m_shapePath.arcToDegrees(Rect::fromSize(right - cr, top, cr, cr), -90, 90); // Top right corner
		m_shapePath.arcToDegrees(Rect::fromSize(right - cr, bottom - cr, cr, cr), 0, 90); // Bottom right corner

		// Arc at the bottom
		m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw, bottom - aw * 2, aw * 2, aw * 2), 90, 90);
		m_shapePath.arcToDegrees(Rect::fromSize(right - rw - aw * 3, bottom - aw * 2, aw * 2, aw * 2), 0, 90);

		m_shapePath.arcToDegrees(Rect::fromSize(left, bottom - cr, cr, cr), 90, 90); // Bottom left corner
		m_shapePath.close();
	}
	else
	{
		m_shapePath.clear();
		m_shapePath.moveTo(Point(left, m_objectArea.center().y));
		m_shapePath.arcToDegrees(Rect::fromSize(left, top, cr, cr), 180, 90); // Top left corner
		m_shapePath.arcToDegrees(Rect::fromSize(right - dockableCircleWidth * 2, top, 
			dockableCircleWidth * 2, m_objectArea.height()), -90, 180); // Right side arc
		m_shapePath.arcToDegrees(Rect::fromSize(left, bottom - cr, cr, cr), 90, 90); // Bottom left corner
		m_shapePath.close();
	}

	m_shapeMesh = m_shapePath.triangulate();
}

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectDrawStruct>();

