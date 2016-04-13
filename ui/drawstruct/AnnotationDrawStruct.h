#ifndef ANNOTATIONDRAWSTRUCT_H
#define ANNOTATIONDRAWSTRUCT_H

#include <ui/drawstruct/ObjectDrawStruct.h>

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

	void drawBackground(QPainter* painter) override;
	void draw(QPainter*, bool) override {}	// Don't draw the normal object
	void drawForeground(QPainter* painter) override;
	void moveVisual(const QPointF& delta) override;
	void update() override;
	bool contains(const QPointF& point) override;

	bool mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	bool acceptsMagneticSnap() const override { return false; }

	virtual QSize getObjectSize();

	void moveText(const QPointF& delta);
	void moveEnd(const QPointF& delta);

protected:
	panda::Annotation* m_annotation;
	QPointF m_endPos, m_startPos;
	QSizeF m_textSize;
	QRectF m_textArea;
	int m_textCounter = -1, m_colorCounter = -1;
	QPainterPath m_shapePath;
	enum MovingAction { MOVING_NONE=0, MOVING_TEXT, MOVING_POINT };
	MovingAction m_movingAction = MOVING_NONE;
	QPointF m_previousMousePos;
};

#endif // ANNOTATIONDRAWSTRUCT_H
