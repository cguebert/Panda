#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <ui/GraphView.h>
#include <ui/Annotation.h>
#include <ui/SimpleDataWidget.h>

#include <QPainter>
#include <QMouseEvent>

namespace panda
{

Annotation::Annotation(PandaDocument *doc)
	: PandaObject(doc)
	, m_type(initData(&m_type, 0, "type", "Type of the annotation"))
	, m_text(initData(&m_text, QString("type text here"), "text", "Text of the annotation"))
	, m_font(initData(&m_font, "font", "Font of the annotation"))
{
	m_type.setWidget("enum_AnnotationTypes");
	m_font.setWidget("font");
}

const char* Annotation::annotationTypes[] = { "Text only", "Arrow", "Rectangle", "Ellipse" };

int AnnotationClass = RegisterObject<Annotation>("Annotation").setName("Annotation").setDescription("Create an annotation in the graph view");

} // namespace panda

//***************************************************************//

using panda::Annotation;

AnnotationDrawStruct::AnnotationDrawStruct(GraphView* view, panda::PandaObject* object)
	: ObjectDrawStruct(view, object)
	, annotation(dynamic_cast<Annotation*>(object))
	, m_textBoxSize(300, 200)
	, m_centerPt(-100, -100)
	, m_endPt(-200, -200)
	, movingAction(MOVING_NONE)
{
	update();
}

void AnnotationDrawStruct::draw(QPainter* painter)
{
	painter->save();
	painter->setBrush(Qt::NoBrush);
	QFont theFont;
	theFont.fromString(annotation->m_font.getValue());
	painter->setFont(theFont);

	QPointF viewDelta = parentView->getViewDelta();

	QPointF ptC, ptE;
	ptC = m_centerPt + position + viewDelta;
	ptE = m_endPt + position + viewDelta;
	switch(annotation->m_type.getValue())
	{
		case Annotation::ANNOTATION_TEXT:
			break;
		case Annotation::ANNOTATION_ARROW:
			break;
		case Annotation::ANNOTATION_RECTANGLE:
		{
			QPointF s2 = ptE - ptC;
			qreal w = fabs(s2.x()), h = fabs(s2.y());
			QRectF rect = QRectF(ptC.x()-w, ptC.y()-h, 2*w, 2*h);
			painter->drawRect(rect);
			break;
		}
		case Annotation::ANNOTATION_ELLIPSE:
		{
			QPointF s2 = ptE - ptC;
			qreal w = fabs(s2.x()), h = fabs(s2.y());
			QRectF rect = QRectF(ptC.x()-w, ptC.y()-h, 2*w, 2*h);
			painter->drawEllipse(rect);
			break;
		}
	}

	const QString& text = annotation->m_text.getValue();
	QRectF tempArea = QRectF(position + viewDelta, m_textBoxSize);
	m_textArea = painter->boundingRect(tempArea, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);
	m_textArea.adjust(-5, -5, 5, 5);

	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(parentView->palette().midlight());
	painter->drawRect(m_textArea);
	painter->restore();

	painter->drawText(tempArea, text);

	const panda::PandaDocument* doc = parentView->getDocument();
	if(doc->getSelection().size() == 1 && doc->isSelected(annotation))	// The annotation is the only selected object
	{
		painter->drawEllipse(ptC, 5, 5);
		painter->drawEllipse(ptE, 5, 5);
	}

	painter->restore();
}

void AnnotationDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
}

bool AnnotationDrawStruct::contains(const QPointF& point)
{
	QPointF ptC, ptE;
	QPointF viewDelta = parentView->getViewDelta();
	ptC = m_centerPt + position + viewDelta;
	ptE = m_endPt + position + viewDelta;

	if((point - ptC).manhattanLength() < 10)
		return true;

	if((point - ptE).manhattanLength() < 10)
		return true;

	return m_textArea.contains(point);
}

void AnnotationDrawStruct::update()
{
	ObjectDrawStruct::update();
}

void AnnotationDrawStruct::save(QDomDocument& doc, QDomElement& elem)
{
	ObjectDrawStruct::save(doc, elem);

	elem.setAttribute("tw", m_textBoxSize.width());
	elem.setAttribute("th", m_textBoxSize.height());

	elem.setAttribute("cx", m_centerPt.x());
	elem.setAttribute("cy", m_centerPt.y());

	elem.setAttribute("ex", m_endPt.x());
	elem.setAttribute("ey", m_endPt.y());
}

void AnnotationDrawStruct::load(QDomElement& elem)
{
	m_textBoxSize.setWidth(elem.attribute("tw").toDouble());
	m_textBoxSize.setHeight(elem.attribute("th").toDouble());

	m_centerPt.setX(elem.attribute("cx").toDouble());
	m_centerPt.setY(elem.attribute("cy").toDouble());

	m_endPt.setX(elem.attribute("ex").toDouble());
	m_endPt.setY(elem.attribute("ey").toDouble());

	ObjectDrawStruct::load(elem);
}

bool AnnotationDrawStruct::mousePressEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / parentView->getZoom();
	QPointF ptC, ptE;
	QPointF viewDelta = parentView->getViewDelta();
	ptC = m_centerPt + position + viewDelta;
	ptE = m_endPt + position + viewDelta;

	if(objectArea.contains(zoomedMouse))
	{
		movingAction = MOVING_TEXT;
		previousMousePos = zoomedMouse;
		return true;
	}

	if((zoomedMouse - ptC).manhattanLength() < 10)
	{
		movingAction = MOVING_CENTER;
		previousMousePos = zoomedMouse;
		return true;
	}

	if((zoomedMouse - ptE).manhattanLength() < 10)
	{
		movingAction = MOVING_END;
		previousMousePos = zoomedMouse;
		return true;
	}

	return false;
}

void AnnotationDrawStruct::mouseMoveEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / parentView->getZoom();
	QPointF delta = zoomedMouse - previousMousePos;
	if(movingAction == MOVING_TEXT)
	{
		move(delta);
		m_centerPt -= delta;
		m_endPt -= delta;
	}
	else if(movingAction == MOVING_CENTER)
	{
		m_centerPt += delta;
		m_endPt += delta;
	}
	else if(movingAction == MOVING_END)
		m_endPt += delta;

	previousMousePos = zoomedMouse;

	parentView->update();
}

void AnnotationDrawStruct::mouseReleaseEvent(QMouseEvent*)
{
	movingAction = MOVING_NONE;
}

QSize AnnotationDrawStruct::getObjectSize()
{
	return m_textBoxSize;
}

int AnnotationDrawClass = RegisterDrawObject<panda::Annotation, AnnotationDrawStruct>();

//*************************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<int, EnumDataWidget<4, panda::Annotation::annotationTypes> > > DWClass_enum_annotation_types("enum_AnnotationTypes",true);
