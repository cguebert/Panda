#include <panda/graphview/object/DockableRenderer.h>

#include <panda/graphview/GraphView.h>
#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/object/ObjectPositionAddon.h>

#include <panda/command/MoveObjectCommand.h>
#include <panda/document/PandaDocument.h>

using panda::types::Point;
using panda::types::Rect;

namespace graphview
{

namespace object
{

DockObjectRenderer::DockObjectRenderer(GraphView* view, panda::DockObject* object)
	: ObjectRenderer(view, object)
	, m_dockObject(object)
{
}

Point DockObjectRenderer::getObjectSize()
{
	Point temp = ObjectRenderer::getObjectSize();
	temp.x += 20;
	temp.y += dockEmptyRendererHeight + dockRendererMargin * 2;

	for (auto dockable : m_dockObject->getDockedObjects())
		temp.y += getParentView()->objectRenderers().get(dockable)->getObjectSize().y + dockRendererMargin;

	return temp;
}

Rect DockObjectRenderer::getTextArea()
{
	float margin = dataRectSize + dataRectMargin + 3;
	Rect textArea = m_visualArea;
	textArea.setHeight(static_cast<float>(ObjectRenderer::objectDefaultHeight));
	textArea.adjust(margin, 0, -margin, 0);
	return textArea;
}

void DockObjectRenderer::placeDockableObjects(bool forceMove)
{
	m_dockablesY.clear();

	float ty = m_visualArea.top() + ObjectRenderer::getObjectSize().y + dockRendererMargin;

	auto doc = getParentView()->document();
	auto& undoStack = doc->getUndoStack();
	bool canMoveObjects = doc->getUndoStack().isInCommandMacro();

	const auto position = getPosition();
	for (auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectRenderer* objectStruct = getParentView()->objectRenderers().get(dockable);
		Point objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();
		Point objectNewPos(position.x + dockHoleWidth - objectSize.x, position.y + ty - m_visualArea.top());

		// If the object has outputs, it is drawn larger but must be placed at the same position
		if (hasOutputs)
			objectNewPos.x += DockableObjectRenderer::dockableWithOutputAdds;

		auto delta = objectNewPos - objectStruct->getPosition();
		if (!delta.isNull())
		{
			if (canMoveObjects)
				undoStack.push(std::make_shared<MoveObjectCommand>(dockable, delta));
			else if (forceMove)
				dockable->addons().edit<ObjectPositionAddon>().move(delta);
		}

		Rect objectArea = objectStruct->getVisualArea();
		m_dockablesY.push_back(objectArea.top());

		ty += objectSize.y + dockRendererMargin;
	}
}

void DockObjectRenderer::createShape()
{
	m_outline.clear();
	m_outline.moveTo(m_visualArea.bottomLeft());
	m_outline.lineTo(m_visualArea.bottomRight());
	m_outline.lineTo(m_visualArea.topRight());
	m_outline.lineTo(m_visualArea.topLeft());

	const float cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const int dhm = dockHoleMargin;
	const int rw = DockableObjectRenderer::dockableWithOutputRect;
	const int aw = DockableObjectRenderer::dockableWithOutputArc;
	const int ah = aw - dockHoleMargin * 2;

	float tx, ty;
	ty = m_visualArea.top() + ObjectRenderer::getObjectSize().y + dockRendererMargin;

	for (auto dockable : m_dockObject->getDockedObjects())
	{
		ObjectRenderer* objectStruct = getParentView()->objectRenderers().get(dockable);
		Point objectSize = objectStruct->getObjectSize();
		bool hasOutputs = !dockable->getOutputDatas().empty();

		tx = m_visualArea.left() + dockHoleWidth - DockableObjectRenderer::dockableCircleWidth + dockHoleMargin;
		float w = DockableObjectRenderer::dockableCircleWidth;
		float h = objectSize.y;

		Rect objectArea = objectStruct->getVisualArea();
		m_outline.lineTo(Point(m_visualArea.left(), ty - dockHoleMargin));
		if (hasOutputs)
		{
			const float top = objectArea.top() - dhm, bot = objectArea.bottom() + dhm;
			const float right = objectArea.right();

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

	ty = m_visualArea.bottom() - dockEmptyRendererHeight - dockRendererMargin;
	m_outline.lineTo(Point(m_visualArea.left(), ty));
	tx = m_visualArea.left() + dockHoleWidth - DockableObjectRenderer::dockableCircleWidth;
	m_outline.arcToDegrees(Rect::fromSize(tx, ty, 
										  static_cast<float>(DockableObjectRenderer::dockableCircleWidth), 
										  static_cast<float>(dockEmptyRendererHeight)),
						   -90, 180);
	m_outline.lineTo(Point(m_visualArea.left(), ty + dockEmptyRendererHeight));
	m_outline.close();

	m_fillShape = m_outline.triangulate();
}

int DockObjectRenderer::getDockableIndex(const Rect& rect)
{
	float y = rect.top();
	int nb = m_dockablesY.size();
	for (int i = 0; i < nb; ++i)
	{
		if (y < m_dockablesY[i])
			return i;
	}
	return -1;
}

int dockObjectDrawClass = RegisterDrawObject<panda::DockObject, DockObjectRenderer>();

//****************************************************************************//

DockableObjectRenderer::DockableObjectRenderer(GraphView* view, panda::DockableObject* dockable)
	: ObjectRenderer(view, dockable)
{
}

bool DockableObjectRenderer::contains(const Point& point)
{
	return m_outline.contains(point);
}

Point DockableObjectRenderer::getObjectSize()
{
	auto size = ObjectRenderer::getObjectSize();
	if (m_hasOutputs)
		size.x += dockableWithOutputAdds;
	return size;
}

Rect DockableObjectRenderer::getTextArea()
{
	auto area = ObjectRenderer::getTextArea();
	if (m_hasOutputs)
		area.adjust(0, 0, -dockableWithOutputAdds, 0);
	return area;
}

void DockableObjectRenderer::update()
{
	m_hasOutputs = !getObject()->getOutputDatas().empty();
	ObjectRenderer::update();
}

void DockableObjectRenderer::createShape()
{
	const float cr = objectCorner * 2; // Rectangle used to create the arc of a corner
	const float rw = dockableWithOutputRect;
	const float aw = dockableWithOutputArc;
	const float left = m_visualArea.left(), top = m_visualArea.top(),
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

int DockableObjectDrawClass = RegisterDrawObject<panda::DockableObject, DockableObjectRenderer>();

} // namespace object

} // namespace graphview
