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
	addInput(&m_type);
	addInput(&m_text);
	addInput(&m_font);

	m_type.setWidget("enum_AnnotationTypes");
	m_font.setWidget("font");
}

void Annotation::setDirtyValue()
{
	PandaObject::setDirtyValue();

	if(!dirtyValue)
		emitModified();
}

const char* Annotation::annotationTypes[] = { "Text only", "Arrow", "Rectangle", "Ellipse" };

int AnnotationClass = RegisterObject<Annotation>("Annotation").setName("Annotation").setDescription("Create an annotation in the graph view");

} // namespace panda

//***************************************************************//

using panda::Annotation;

AnnotationDrawStruct::AnnotationDrawStruct(GraphView* view, panda::PandaObject* object)
	: ObjectDrawStruct(view, object)
	, annotation(dynamic_cast<Annotation*>(object))
	, m_deltaToEnd(200, 100)
	, m_textCounter(-1)
	, movingAction(MOVING_NONE)
{
	update();
}

void AnnotationDrawStruct::drawBackground(QPainter* painter)
{
	painter->save();
	painter->setBrush(Qt::NoBrush);

	const QString& text = annotation->m_text.getValue();
	int textCounter = annotation->m_text.getCounter();
	if(m_textSize.isEmpty() || m_textCounter != textCounter)
	{
		QFont theFont;
		theFont.fromString(annotation->m_font.getValue());
		painter->setFont(theFont);

		QRectF tempArea = QRectF(m_startPos, QSizeF(1000, 1000));
		tempArea = painter->boundingRect(tempArea, Qt::AlignLeft | Qt::AlignTop, text);
		m_textSize = tempArea.size();
		m_textCounter = textCounter;
		update();
	}

	switch(annotation->m_type.getValue())
	{
		case Annotation::ANNOTATION_TEXT:
			break;
		case Annotation::ANNOTATION_ARROW:
			break;
		case Annotation::ANNOTATION_RECTANGLE:
		{
			QRectF rect = QRectF(m_startPos, m_endPos);
			painter->drawRect(rect);
			break;
		}
		case Annotation::ANNOTATION_ELLIPSE:
		{
			QRectF rect = QRectF(m_startPos, m_endPos);
			painter->drawEllipse(rect);
			break;
		}
	}

	painter->restore();
}

void AnnotationDrawStruct::drawForeground(QPainter* painter)
{
	painter->save();
	painter->setBrush(Qt::NoBrush);
	QFont theFont;
	theFont.fromString(annotation->m_font.getValue());
	painter->setFont(theFont);

	const QString& text = annotation->m_text.getValue();

	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(parentView->palette().midlight());
	painter->drawRect(m_textArea);
	painter->restore();

	painter->drawText(m_textArea.adjusted(5, 5, -5, -5), Qt::AlignLeft | Qt::AlignTop, text);

	const panda::PandaDocument* doc = parentView->getDocument();
	if(annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& doc->getSelection().size() == 1
			&& doc->isSelected(annotation))	// The annotation is the only selected object
		painter->drawEllipse(m_endPos, 5, 5);

	painter->restore();
}

void AnnotationDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);

	m_textArea.translate(delta);
	m_startPos += delta;
	m_endPos += delta;
}

bool AnnotationDrawStruct::contains(const QPointF& point)
{
	const panda::PandaDocument* doc = parentView->getDocument();
	if(annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& doc->getSelection().size() == 1
			&& doc->isSelected(annotation))	// The annotation is the only selected object
	{
		if((point - m_endPos).manhattanLength() < 10)
			return true;
	}

	return m_textArea.contains(point);
}

void AnnotationDrawStruct::update()
{
//	ObjectDrawStruct::update();

	QPointF viewDelta = parentView->getViewDelta();

	m_startPos = position + viewDelta;
	m_endPos = m_startPos + m_deltaToEnd;

	m_textArea = QRectF(m_startPos, m_textSize);
	m_textArea.translate(0, -m_textSize.height());
	m_textArea.adjust(3, -13, 13, -3);

	annotation->cleanDirty();
}

void AnnotationDrawStruct::save(QDomDocument& doc, QDomElement& elem)
{
	ObjectDrawStruct::save(doc, elem);

	elem.setAttribute("dx", m_deltaToEnd.x());
	elem.setAttribute("dy", m_deltaToEnd.y());
}

void AnnotationDrawStruct::load(QDomElement& elem)
{
	m_deltaToEnd.setX(elem.attribute("dx").toDouble());
	m_deltaToEnd.setY(elem.attribute("dy").toDouble());

	ObjectDrawStruct::load(elem);
}

bool AnnotationDrawStruct::mousePressEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / parentView->getZoom();

	if(m_textArea.contains(zoomedMouse))
	{
		movingAction = MOVING_TEXT;
		previousMousePos = zoomedMouse;
		return true;
	}

	if((zoomedMouse - m_endPos).manhattanLength() < 10)
	{
		movingAction = MOVING_POINT;
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
		emit parentView->modified();
		parentView->update();
	}
	else if(movingAction == MOVING_POINT)
	{
		m_deltaToEnd += delta;
		m_endPos += delta;
		emit parentView->modified();
		parentView->update();
	}

	previousMousePos = zoomedMouse;
}

void AnnotationDrawStruct::mouseReleaseEvent(QMouseEvent*)
{
	movingAction = MOVING_NONE;
}

QSize AnnotationDrawStruct::getObjectSize()
{
	return QSize(100, 50);
}

int AnnotationDrawClass = RegisterDrawObject<panda::Annotation, AnnotationDrawStruct>();

//*************************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<int, EnumDataWidget<4, panda::Annotation::annotationTypes> > > DWClass_enum_annotation_types("enum_AnnotationTypes",true);
