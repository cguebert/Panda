#pragma once

#include <ui/graphview/object/ObjectRenderer.h>

class GraphView;

namespace panda
{
class PandaObject;
class Annotation;
}

//****************************************************************************//

namespace graphview
{

namespace object
{

class AnnotationRenderer : public ObjectRenderer
{
public:
	AnnotationRenderer(GraphView* view, panda::PandaObject* m_object);

	void update() override;
	void createShape() override;
	void move(const panda::types::Point& delta) override;
	bool contains(const panda::types::Point& point) override;

	bool mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	bool acceptsMagneticSnap() const override { return false; }

	virtual panda::types::Point getObjectSize();

	void moveText(const panda::types::Point& delta);
	void moveEnd(const panda::types::Point& delta);

protected:
	void drawBackground(graphics::DrawList& list, graphics::DrawColors& colors) override;
	void draw(graphics::DrawList& list, graphics::DrawColors& colors, bool selected) override {}	// Don't draw the normal object
	void drawForeground(graphics::DrawList& list, graphics::DrawColors& colors) override;

	void deltaToEndChanged();

	panda::Annotation* m_annotation;
	panda::types::Point m_endPos, m_textSize;
	panda::types::Rect m_textArea;
	int m_textCounter = -1, m_colorCounter = -1;
	enum MovingAction { MOVING_NONE=0, MOVING_TEXT, MOVING_POINT };
	MovingAction m_movingAction = MOVING_NONE;
	panda::types::Point m_previousMousePos, m_startMousePos;
};

} // namespace object

} // namespace graphview
