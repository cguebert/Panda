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
	Data<QPointF> m_centerPt, m_endPt;
	Data<QRectF> m_textArea;
	Data<QString> m_text, m_font;
//	Data<QColor> m_color;

	static const char* annotationTypes[];
};

} // namespace panda

//***************************************************************//

class AnnotationDrawStruct : public ObjectDrawStruct
{
public:
	AnnotationDrawStruct(GraphView* view, panda::PandaObject* object);

	virtual void drawShape(QPainter* painter);
	virtual void drawText(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);
};

#endif // ANNOTATION_H
