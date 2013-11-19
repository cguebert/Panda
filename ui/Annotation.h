#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <panda/PandaObject.h>

#include <ui/ObjectDrawStruct.h>

class GraphView;

namespace panda
{

class Annotation : public PandaObject
{
public:
	PANDA_CLASS(Annotation, PandaObject)

	Annotation(PandaDocument *doc);

	virtual void setDirtyValue();

	Data<int> m_type;
	Data<QString> m_text, m_font;

	enum AnnotationType { ANNOTATION_TEXT=0, ANNOTATION_ARROW, ANNOTATION_RECTANGLE, ANNOTATION_ELLIPSE };
	static const char* annotationTypes[];
};

} // namespace panda

//***************************************************************//

class AnnotationDrawStruct : public ObjectDrawStruct
{
public:
	AnnotationDrawStruct(GraphView* view, panda::PandaObject* object);

	virtual void drawBackground(QPainter* painter);
	virtual void draw(QPainter*) {}	// Don't draw the normal object
	virtual void drawForeground(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);

	virtual void save(QDomDocument& doc, QDomElement& elem);
	virtual void load(QDomElement& elem);

	virtual bool mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

	virtual QSize getObjectSize();

protected:
	panda::Annotation* annotation;
	QPointF m_deltaToEnd, m_endPos, m_startPos;
	QSizeF m_textSize;
	QRectF m_textArea;
	int m_textCounter;
	QPainterPath shapePath;
	enum MovingAction { MOVING_NONE=0, MOVING_TEXT, MOVING_POINT };
	MovingAction movingAction;
	QPointF previousMousePos;
};

#endif // ANNOTATION_H
