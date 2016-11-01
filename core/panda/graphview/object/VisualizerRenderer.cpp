#include <panda/graphview/object/VisualizerRenderer.h>

#include <panda/graphview/GraphView.h>
#include <panda/graphview/InteractionEvents.h>
#include <panda/graphview/Viewport.h>
#include <panda/graphview/object/ObjectRendererFactory.h>

#include <panda/command/MoveObjectCommand.h>
#include <panda/command/SetDataValueCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/object/Visualizer.h>
#include <panda/helper/algorithm.h>

namespace panda
{

using types::Point;
using types::Rect;

namespace graphview
{

namespace object
{

VisualizerRenderer::VisualizerRenderer(GraphView* view, PandaObject* object)
	: ObjectRenderer(view, object)
	, m_visualizer(dynamic_cast<Visualizer*>(object))
{
}

void VisualizerRenderer::drawForeground(graphics::DrawList& list, graphics::DrawColors& colors)
{
	list.addMesh(m_fillShape, colors.lightColor);
	list.addPolyline(m_outline, colors.penColor, false);
}

void VisualizerRenderer::move(const Point& delta)
{
	ObjectRenderer::move(delta);
	m_visualizerArea.translate(delta);
}

void VisualizerRenderer::resize(const types::Point& delta)
{
	auto size = m_visualizerArea.size() + delta;
	size.x = std::max(20.f, size.x);
	size.y = std::max(20.f, size.y);

	m_visualizerArea = Rect::fromSize(getPosition(), size);
	m_selectionArea = m_visualArea = m_visualizerArea;
	createShape();
}

void VisualizerRenderer::update()
{
//	ObjectRenderer::update();	// No need to call it

	if (m_mouseAction == Action::None)
		m_visualizerArea = Rect::fromSize(getPosition(), m_visualizer->visualizerSize.getValue());

	m_selectionArea = m_visualArea = m_visualizerArea;

	createShape();
}

void VisualizerRenderer::createShape()
{
	m_outline.clear();

	m_outline.rect(m_visualizerArea);
	m_outline.close();

	m_fillShape = m_outline.triangulate();
}

bool VisualizerRenderer::mousePressEvent(const MouseEvent& event)
{
	Point zoomedMouse = getParentView()->viewport().toView(event.pos());

	auto size = m_visualizerArea.size() / 4;
	helper::bound(15.f, size.x, 50.f);
	helper::bound(15.f, size.y, 50.f);
	Rect resizeHandle = Rect::fromSize(m_visualizerArea.bottomRight() - size, size);

	m_startMousePos = m_previousMousePos = zoomedMouse;

	if(resizeHandle.contains(zoomedMouse))
		m_mouseAction = Action::Resizing;
	else
		m_mouseAction = Action::Moving;

	return true;
}

void VisualizerRenderer::mouseMoveEvent(const MouseEvent& event)
{
	Point zoomedMouse = getParentView()->viewport().toView(event.pos());
	Point delta = zoomedMouse - m_previousMousePos;
	m_previousMousePos = zoomedMouse;
	if(delta.isNull())
		return;

	if (m_mouseAction == Action::Moving)
		move(delta);
	else if (m_mouseAction == Action::Resizing)
		resize(delta);

	getParentView()->update();
}

void VisualizerRenderer::mouseReleaseEvent(const MouseEvent& event)
{
	Point zoomedMouse = getParentView()->viewport().toView(event.pos());
	Point deltaStart = m_startMousePos - m_previousMousePos;
	Point delta = zoomedMouse - m_startMousePos;

	if (m_mouseAction == Action::Moving)
	{
		move(deltaStart);
		getParentView()->document()->getUndoStack().push(std::make_shared<MoveObjectCommand>(m_visualizer, delta));
	}
	else if (m_mouseAction == Action::Resizing)
	{
		const auto prevSize = m_visualizer->visualizerSize.getValue();
		const auto newSize = m_visualizerArea.size();

		if(prevSize != newSize)
			getParentView()->document()->getUndoStack().push(std::make_shared<SetDataValueCommand<Point>>(&m_visualizer->visualizerSize, prevSize, newSize));
	}
		
	m_mouseAction = Action::None;
}

Point VisualizerRenderer::getObjectSize()
{
	return m_visualizer->visualizerSize.getValue();
}

int VisualizerDrawClass = RegisterDrawObject<Visualizer, VisualizerRenderer>();

} // namespace object

} // namespace graphview

} // namespace panda
