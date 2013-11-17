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

	Data<int> m_type;

	Data<QString> m_text, m_font;
//	Data<QColor> m_color;

	enum AnnotationType { ANNOTATION_TEXT=0, ANNOTATION_ARROW, ANNOTATION_RECTANGLE, ANNOTATION_ELLIPSE };
	static const char* annotationTypes[];
};

} // namespace panda

//***************************************************************//

class AnnotationDrawStruct : public ObjectDrawStruct
{
public:
	AnnotationDrawStruct(GraphView* view, panda::PandaObject* object);

	virtual void draw(QPainter* painter);
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
	QPointF m_centerPt, m_endPt;
	QSize m_textBoxSize;
	QRectF m_textArea;
	enum MovingAction { MOVING_NONE=0, MOVING_TEXT, MOVING_CENTER, MOVING_END };
	MovingAction movingAction;
	QPointF previousMousePos;
};

#endif // ANNOTATION_H
