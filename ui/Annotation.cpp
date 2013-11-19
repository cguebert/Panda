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
	if(!dirtyValue)
	{
		PandaObject::setDirtyValue();

		emitModified();
	}
	else
		PandaObject::setDirtyValue();
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

	// Compute the bounding box of the text, if it changed
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

	// Draw the shape of the annotation
	painter->setBrush(parentView->palette().light());
	painter->drawPath(shapePath);

	painter->restore();
}

void AnnotationDrawStruct::drawForeground(QPainter* painter)
{
	painter->save();
	painter->setBrush(Qt::NoBrush);
	QFont theFont;
	theFont.fromString(annotation->m_font.getValue());
	painter->setFont(theFont);

	// Draw the box behind the text
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(parentView->palette().midlight());
	painter->drawRect(m_textArea);
	painter->restore();

	// Draw the text
	painter->drawText(m_textArea.adjusted(5, 5, -5, -5), Qt::AlignLeft | Qt::AlignTop, annotation->m_text.getValue());

	// Draw the handle
	const panda::PandaDocument* doc = parentView->getDocument();
	if(annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& doc->getSelection().size() == 1
			&& doc->isSelected(annotation))	// The annotation is the only selected object
	{
		painter->setBrush(parentView->palette().midlight());
		painter->drawEllipse(m_endPos, 5, 5);
	}

	painter->restore();
}

void AnnotationDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);

	m_textArea.translate(delta);
	shapePath.translate(delta);
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
//	ObjectDrawStruct::update();	// No need to call it

	QPointF viewDelta = parentView->getViewDelta();

	m_startPos = position + viewDelta;
	m_endPos = m_startPos + m_deltaToEnd;

	m_textArea = QRectF(m_startPos, m_textSize);
	m_textArea.translate(0, -m_textSize.height());
	m_textArea.adjust(3, -13, 13, -3);

	shapePath = QPainterPath();
	switch(annotation->m_type.getValue())
	{
		case Annotation::ANNOTATION_TEXT:
			break;
		case Annotation::ANNOTATION_ARROW:
		{
			QPointF start = m_textArea.center();
			QPointF dir = m_endPos - start;
			const qreal w = 2.5;
			qreal length = sqrt(dir.x()*dir.x() + dir.y()*dir.y());

			if(length < w * 20)
				break;

			dir /= length;
			QPointF dir2 = QPointF(-dir.y(), dir.x());
			dir *= w;
			dir2 *= w;

			shapePath.moveTo(m_endPos);
			shapePath.lineTo(m_endPos + 5*dir2 - 15*dir);
			shapePath.lineTo(m_endPos + dir2 - 10*dir);
			shapePath.lineTo(start + dir2);
			shapePath.lineTo(start - dir2);
			shapePath.lineTo(m_endPos - dir2 - 10*dir);
			shapePath.lineTo(m_endPos - 5*dir2 - 15*dir);
			shapePath.lineTo(m_endPos);
			break;
		}
		case Annotation::ANNOTATION_RECTANGLE:
		{
			QRectF rect = QRectF(m_startPos, m_endPos);
			shapePath.addRect(rect);
			break;
		}
		case Annotation::ANNOTATION_ELLIPSE:
		{
			QRectF rect = QRectF(m_startPos, m_endPos);
			shapePath.addEllipse(rect);
			break;
		}
	}

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
	previousMousePos = zoomedMouse;
	if(delta.isNull())
		return;

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
		update();
		emit parentView->modified();
		parentView->update();
	}
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
