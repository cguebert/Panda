#ifndef ANNOTATIONDRAWSTRUCT_H
#define ANNOTATIONDRAWSTRUCT_H

#include <ui/graphview/object/ObjectDrawStruct.h>

class GraphView;

namespace panda
{
class PandaObject;
class Annotation;
}

//****************************************************************************//

class AnnotationDrawStruct : public ObjectDrawStruct
{
public:
	AnnotationDrawStruct(GraphView* view, panda::PandaObject* m_object);

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
	void drawBackground(DrawList& list, DrawColors& colors) override;
	void draw(DrawList& list, DrawColors& colors, bool selected) override {}	// Don't draw the normal object
	void drawForeground(DrawList& list, DrawColors& colors) override;

	void deltaToEndChanged();

	panda::Annotation* m_annotation;
	panda::types::Point m_endPos, m_textSize;
	panda::types::Rect m_textArea;
	int m_textCounter = -1, m_colorCounter = -1;
	enum MovingAction { MOVING_NONE=0, MOVING_TEXT, MOVING_POINT };
	MovingAction m_movingAction = MOVING_NONE;
	panda::types::Point m_previousMousePos, m_startMousePos;
};

#endif // ANNOTATIONDRAWSTRUCT_H
