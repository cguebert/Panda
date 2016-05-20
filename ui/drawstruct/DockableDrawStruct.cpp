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

Point DockObjectDrawStruct::getObjectSize()
{
	Point temp = ObjectDrawStruct::getObjectSize();
	temp.x += 20;
	temp.y += dockEmptyRendererHeight + dockRendererMargin * 2;

	for(auto dockable : m_dockObject->getDockedObjects())
		temp.y += getParentView()->getObjectDrawStruct(dockable)->getObjectSize().y + dockRendererMargin;

	return temp;
}

Rect DockObjectDrawStruct::getTextArea()
{
	int margin = dataRectSize+dataRectMargin+3;
	Rect textArea = m_visualArea;
	textArea.setHeight(ObjectDrawStruct::objectDefaultHeight);
	textArea.adjust(margin, 0, -margin, 0);
	return textArea;
}

void DockObjectDrawStruct::move(const Point& delta)
{
	ObjectDrawStruct::move(delta);
	for(auto dockable : m_dockObject->getDockedObjects())
		getParentView()->getObjectDrawStruct(dockable)->move(delta);
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
	ty = m_visualArea.top() + ObjectDrawStruct::getObjectSize().y + dockRendererMargin;

	auto doc = getParentView()->getDocument();
	auto& undoStack = doc->getUndoStack();
	bool canMoveObjects = doc->getUndoStack().isInCommandMacro();

	const auto position = getPosition();
	for (auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectDrawStruct* objectStruct = getParentView()->getObjectDrawStruct(dockable);
		Point objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();
		Point objectNewPos(position.x + dockHoleWidth - objectSize.x, position.y + ty - m_visualArea.top());

		// If the object has outputs, it is drawn larger but must be placed at the same position
		if (hasOutputs)
			objectNewPos.x += DockableObjectDrawStruct::dockableWithOutputAdds;

		if (canMoveObjects && objectNewPos != objectStruct->getPosition())
			undoStack.push(std::make_shared<MoveObjectCommand>(dockable, objectNewPos - objectStruct->getPosition()));

		Rect objectArea = objectStruct->getVisualArea();
		m_dockablesY.push_back(objectArea.top());

		ty += objectSize.y + dockRendererMargin;
	}
}

void DockObjectDrawStruct::createShape()
{
	m_outline.clear();
	m_outline.moveTo(m_visualArea.bottomLeft());
	m_outline.lineTo(m_visualArea.bottomRight());
	m_outline.lineTo(m_visualArea.topRight());
	m_outline.lineTo(m_visualArea.topLeft());

	const int cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int dhm = dockHoleMargin;
	const int rw = DockableObjectDrawStruct::dockableWithOutputRect;
	const int aw = DockableObjectDrawStruct::dockableWithOutputArc;
	const int ah = aw - dockHoleMargin * 2;

	int tx, ty;
	ty = m_visualArea.top() + ObjectDrawStruct::getObjectSize().y + dockRendererMargin;

	for(auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectDrawStruct* objectStruct = getParentView()->getObjectDrawStruct(dockable);
		Point objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();

		tx = m_visualArea.left() + dockHoleWidth - DockableObjectDrawStruct::dockableCircleWidth + dockHoleMargin;
		int w = DockableObjectDrawStruct::dockableCircleWidth;
		int h = objectSize.y;

		Rect objectArea = objectStruct->getVisualArea();
		m_outline.lineTo(Point(m_visualArea.left(), ty - dockHoleMargin));
		if (hasOutputs)
		{
			const int top = objectArea.top() - dhm, bot = objectArea.bottom() + dhm;
			const auto right = objectArea.right();

			// Arc at the top
			m_outline.arcToDegrees(Rect::fromSize(right - rw - aw * 3, top, aw * 2, ah * 2), -90, 90);
			m_outline.arcToDegrees(Rect::fromSize(right - rw - aw, top, aw * 2, ah * 2), 180, 90);

			m_outline.arcToDegrees(Rect::fromSize(right - cr + dhm, top, cr, cr), -90, 90); // Top right corner
			m_outline.arcToDegrees(Rect::fromSize(right - cr + dhm, bot - cr, cr, cr), 0, 90); // Bottom right corner

			// Arc at the bottom
			m_outline.arcToDegrees(Rect::fromSize(right - rw - aw, bot - ah * 2, aw * 2, ah * 2), 90, 90);
			m_outline.arcToDegrees(Rect::fromSize(right - rw - aw * 3, bot - ah * 2, aw * 2, ah * 2), 0, 90);
		}
		else
			m_outline.arcToDegrees(Rect::fromSize(tx - w - dockHoleMargin, ty - dockHoleMargin, w * 2 + dockHoleMargin, h + dockHoleMargin * 2), -90, 180);
		m_outline.lineTo(Point(m_visualArea.left(), ty + h + dockHoleMargin));

		ty += h + dockRendererMargin;
	}

	ty = m_visualArea.bottom()-dockEmptyRendererHeight-dockRendererMargin;
	m_outline.lineTo(Point(m_visualArea.left(), ty));
	tx = m_visualArea.left()+dockHoleWidth-DockableObjectDrawStruct::dockableCircleWidth;
	m_outline.arcToDegrees(Rect::fromSize(tx, ty, DockableObjectDrawStruct::dockableCircleWidth, dockEmptyRendererHeight), -90, 180);
	m_outline.lineTo(Point(m_visualArea.left(), ty+dockEmptyRendererHeight));
	m_outline.close();

	m_fillShape = m_outline.triangulate();
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

bool DockableObjectDrawStruct::contains(const Point& point)
{
	return m_outline.contains(point);
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
}

void DockableObjectDrawStruct::createShape()
{
	const int cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int rw = dockableWithOutputRect;
	const int aw = dockableWithOutputArc;
	const auto left = m_visualArea.left(), top = m_visualArea.top(), 
		right = m_visualArea.right(), bottom = m_visualArea.bottom();

	if (m_hasOutputs)
	{
		m_outline.clear();
		m_outline.moveTo(Point(left, m_visualArea.center().y));
		m_outline.arcToDegrees(Rect::fromSize(left, top, cr, cr), 180, 90); // Top left corner

		// Arc at the top
		m_outline.arcToDegrees(Rect::fromSize(right - rw - aw * 3, top, aw * 2, aw * 2), -90, 90);
		m_outline.arcToDegrees(Rect::fromSize(right - rw - aw, top, aw * 2, aw * 2), 180, 90);

		m_outline.arcToDegrees(Rect::fromSize(right - cr, top, cr, cr), -90, 90); // Top right corner
		m_outline.arcToDegrees(Rect::fromSize(right - cr, bottom - cr, cr, cr), 0, 90); // Bottom right corner

		// Arc at the bottom
		m_outline.arcToDegrees(Rect::fromSize(right - rw - aw, bottom - aw * 2, aw * 2, aw * 2), 90, 90);
		m_outline.arcToDegrees(Rect::fromSize(right - rw - aw * 3, bottom - aw * 2, aw * 2, aw * 2), 0, 90);

		m_outline.arcToDegrees(Rect::fromSize(left, bottom - cr, cr, cr), 90, 90); // Bottom left corner
		m_outline.close();
	}
	else
	{
		m_outline.clear();
		m_outline.moveTo(Point(left, m_visualArea.center().y));
		m_outline.arcToDegrees(Rect::fromSize(left, top, cr, cr), 180, 90); // Top left corner
		m_outline.arcToDegrees(Rect::fromSize(right - dockableCircleWidth * 2, top, 
			dockableCircleWidth * 2, m_visualArea.height()), -90, 180); // Right side arc
		m_outline.arcToDegrees(Rect::fromSize(left, bottom - cr, cr, cr), 90, 90); // Bottom left corner
		m_outline.close();
	}

	m_fillShape = m_outline.triangulate();
}

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectDrawStruct>();

