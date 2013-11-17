#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <ui/GraphView.h>
#include <ui/Annotation.h>
#include <ui/SimpleDataWidget.h>

#include <QPainter>

namespace panda
{

Annotation::Annotation(PandaDocument *doc)
	: PandaObject(doc)
	, m_type(initData(&m_type, 0, "type", "Type of the annotation"))
	, m_centerPt(initData(&m_centerPt, "center", "Center of the shape"))
	, m_endPt(initData(&m_endPt, "end", "Other end of the shape"))
	, m_textArea(initData(&m_textArea, "area", "Rectangle in which to draw the text"))
	, m_text(initData(&m_text, "text", "Text of the annotation"))
	, m_font(initData(&m_font, "font", "Font of the annotation"))
{
	m_type.setWidget("enum_AnnotationTypes");
	m_font.setWidget("font");
}

const char* Annotation::annotationTypes[] = { "Text only", "Arrow", "Rectangle", "Ellipse" };

int AnnotationClass = RegisterObject<Annotation>("Annotation").setName("Annotation").setDescription("Create an annotation in the graph view");

} // namespace panda

//***************************************************************//

AnnotationDrawStruct::AnnotationDrawStruct(GraphView* view, panda::PandaObject* object)
	: ObjectDrawStruct(view, object)
{
	update();
}

void AnnotationDrawStruct::drawShape(QPainter* painter)
{
//	painter->drawPath(shapePath);
}

void AnnotationDrawStruct::drawText(QPainter* painter)
{
	ObjectDrawStruct::drawText(painter);
}

void AnnotationDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
}

bool AnnotationDrawStruct::contains(const QPointF& point)
{
	return ObjectDrawStruct::contains(point);
}

void AnnotationDrawStruct::update()
{
	ObjectDrawStruct::update();
}

int AnnotationDrawClass = RegisterDrawObject<panda::Annotation, AnnotationDrawStruct>();

//*************************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<int, EnumDataWidget<4, panda::Annotation::annotationTypes> > > DWClass_enum_annotation_types("enum_AnnotationTypes",true);
