#pragma once

#include <panda/graphview/object/ObjectRenderer.h>

namespace panda
{
class PandaObject;
class Visualizer;

namespace graphview
{

class GraphView;

namespace object
{

class VisualizerRenderer : public ObjectRenderer
{
public:
	VisualizerRenderer(GraphView* view, PandaObject* m_object);

	void update() override;
	void createShape() override;
	void move(const types::Point& delta) override;

	bool mousePressEvent(const MouseEvent& event) override;
	void mouseMoveEvent(const MouseEvent& event) override;
	void mouseReleaseEvent(const MouseEvent& event) override;

	bool acceptsMagneticSnap() const override { return false; }

	types::Point getObjectSize() override;

	void resize(const types::Point& delta);

protected:
	void drawBackground(graphics::DrawList& list, graphics::DrawColors& colors) override {}
	void draw(graphics::DrawList& list, graphics::DrawColors& colors, bool selected) override {}	// Don't draw the normal object
	void drawForeground(graphics::DrawList& list, graphics::DrawColors& colors) override;

	Visualizer* m_visualizer;
	types::Rect m_visualizerArea;
	enum class Action { None, Moving, Resizing };
	Action m_mouseAction = Action::None;
	types::Point m_previousMousePos, m_startMousePos;
};

} // namespace object

} // namespace graphview

} // namespace panda
