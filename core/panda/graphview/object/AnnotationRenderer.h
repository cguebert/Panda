#pragma once

#include <panda/graphview/object/ObjectRenderer.h>

namespace panda
{
class PandaObject;
class Annotation;

namespace graphview
{

class GraphView;

namespace object
{

class AnnotationRenderer : public ObjectRenderer
{
public:
	AnnotationRenderer(GraphView* view, PandaObject* m_object);

	void update() override;
	void createShape() override;
	void move(const types::Point& delta) override;
	bool contains(const types::Point& point) override;

	bool mousePressEvent(const MouseEvent& event) override;
	void mouseMoveEvent(const MouseEvent& event) override;
	void mouseReleaseEvent(const MouseEvent& event) override;

	bool acceptsMagneticSnap() const override { return false; }

	virtual types::Point getObjectSize();

	void moveText(const types::Point& delta);
	void moveEnd(const types::Point& delta);

protected:
	void drawBackground(graphics::DrawList& list, graphics::DrawColors& colors) override;
	void draw(graphics::DrawList& list, graphics::DrawColors& colors, bool selected) override {}	// Don't draw the normal object
	void drawForeground(graphics::DrawList& list, graphics::DrawColors& colors) override;

	void deltaToEndChanged();

	Annotation* m_annotation;
	types::Point m_endPos, m_textSize;
	types::Rect m_textArea;
	int m_textCounter = -1, m_colorCounter = -1;
	enum MovingAction { MOVING_NONE=0, MOVING_TEXT, MOVING_POINT };
	MovingAction m_movingAction = MOVING_NONE;
	types::Point m_previousMousePos, m_startMousePos;
};

} // namespace object

} // namespace graphview

} // namespace panda
