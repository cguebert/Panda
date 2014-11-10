#include <ui/GraphView.h>
#include <ui/Annotation.h>
#include <ui/drawstruct/AnnotationDrawStruct.h>
#include <ui/command/ModifyAnnotationCommand.h>

#include <panda/PandaDocument.h>

#include <QPainter>
#include <QMouseEvent>

using panda::Annotation;

AnnotationDrawStruct::AnnotationDrawStruct(GraphView* view, panda::PandaObject* object)
	: ObjectDrawStruct(view, object)
	, m_annotation(dynamic_cast<Annotation*>(object))
	, m_deltaToEnd(200, 100)
	, m_textCounter(-1)
	, m_fontCounter(-1)
	, m_movingAction(MOVING_NONE)
{
	update();
}

void AnnotationDrawStruct::drawBackground(QPainter* painter)
{
	painter->save();
	painter->setBrush(Qt::NoBrush);

	// Compute the bounding box of the text, if it changed
	const QString& text = m_annotation->m_text.getValue();
	int textCounter = m_annotation->m_text.getCounter();
	int fontCounter = m_annotation->m_font.getCounter();
	if(m_textSize.isEmpty() || m_textCounter != textCounter || m_fontCounter != fontCounter)
	{
		QFont theFont;
		theFont.fromString(m_annotation->m_font.getValue());
		painter->setFont(theFont);

		QRectF tempArea = QRectF(m_startPos, QSizeF(1000, 1000));
		tempArea = painter->boundingRect(tempArea, Qt::AlignLeft | Qt::AlignTop, text);
		m_textSize = tempArea.size();
		m_textCounter = textCounter;
		m_fontCounter = fontCounter;
		update();
	}

	// Draw the shape of the annotation
	painter->setBrush(m_parentView->palette().light());
	painter->drawPath(m_shapePath);

	painter->restore();
}

void AnnotationDrawStruct::drawForeground(QPainter* painter)
{
	painter->save();
	painter->setBrush(Qt::NoBrush);
	QFont theFont;
	theFont.fromString(m_annotation->m_font.getValue());
	painter->setFont(theFont);

	// Draw the box behind the text
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(m_parentView->palette().midlight());
	painter->drawRect(m_textArea);
	painter->restore();

	// Draw the text
	painter->drawText(m_textArea.adjusted(5, 5, -5, -5), Qt::AlignLeft | Qt::AlignTop, m_annotation->m_text.getValue());

	// Draw the handle
	const panda::PandaDocument* doc = m_parentView->getDocument();
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& doc->getSelection().size() == 1
			&& doc->isSelected(m_annotation))	// The annotation is the only selected object
	{
		painter->setBrush(m_parentView->palette().midlight());
		painter->drawEllipse(m_endPos, 5, 5);
	}

	painter->restore();
}

void AnnotationDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);

	m_textArea.translate(delta);
	m_shapePath.translate(delta);
	m_startPos += delta;
	m_endPos += delta;
}

void AnnotationDrawStruct::moveText(const QPointF& delta)
{
	move(delta);
	emit m_parentView->modified();
	m_parentView->update();
}

void AnnotationDrawStruct::moveEnd(const QPointF& delta)
{
	m_deltaToEnd += delta;
	m_endPos += delta;
	update();
	emit m_parentView->modified();
	m_parentView->update();
}

bool AnnotationDrawStruct::contains(const QPointF& point)
{
	const panda::PandaDocument* doc = m_parentView->getDocument();
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& doc->getSelection().size() == 1
			&& doc->isSelected(m_annotation))	// The annotation is the only selected object
	{
		if((point - m_endPos).manhattanLength() < 10)
			return true;
	}

	return m_textArea.contains(point);
}

void AnnotationDrawStruct::update()
{
//	ObjectDrawStruct::update();	// No need to call it

	QPointF viewDelta = m_parentView->getViewDelta();

	m_startPos = m_position + viewDelta;
	m_endPos = m_startPos + m_deltaToEnd;

	m_textArea = QRectF(m_startPos, m_textSize);
	m_textArea.translate(0, -m_textSize.height());
	m_textArea.adjust(3, -13, 13, -3);

	m_shapePath = QPainterPath();
	switch(m_annotation->m_type.getValue())
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

			m_shapePath.moveTo(m_endPos);
			m_shapePath.lineTo(m_endPos + 5*dir2 - 15*dir);
			m_shapePath.lineTo(m_endPos + dir2 - 10*dir);
			m_shapePath.lineTo(start + dir2);
			m_shapePath.lineTo(start - dir2);
			m_shapePath.lineTo(m_endPos - dir2 - 10*dir);
			m_shapePath.lineTo(m_endPos - 5*dir2 - 15*dir);
			m_shapePath.lineTo(m_endPos);
			break;
		}
		case Annotation::ANNOTATION_RECTANGLE:
		{
			QRectF rect = QRectF(m_startPos, m_endPos);
			m_shapePath.addRect(rect);
			break;
		}
		case Annotation::ANNOTATION_ELLIPSE:
		{
			QRectF rect = QRectF(m_startPos, m_endPos);
			m_shapePath.addEllipse(rect);
			break;
		}
	}

	m_annotation->cleanDirty();
	m_objectArea = m_textArea;
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT)
		 m_objectArea |= QRectF(m_startPos, m_endPos).normalized();
}

void AnnotationDrawStruct::save(QDomDocument& doc, QDomElement& elem)
{
	ObjectDrawStruct::save(doc, elem);

	elem.setAttribute("dx", m_deltaToEnd.x());
	elem.setAttribute("dy", m_deltaToEnd.y());
}

void AnnotationDrawStruct::load(QDomElement& elem)
{
#ifdef PANDA_DOUBLE
	m_deltaToEnd.setX(elem.attribute("dx").toDouble());
	m_deltaToEnd.setY(elem.attribute("dy").toDouble());
#else
	m_deltaToEnd.setX(elem.attribute("dx").toFloat());
	m_deltaToEnd.setY(elem.attribute("dy").toFloat());
#endif

	ObjectDrawStruct::load(elem);
}

bool AnnotationDrawStruct::mousePressEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / m_parentView->getZoom();

	if(m_textArea.contains(zoomedMouse))
	{
		m_movingAction = MOVING_TEXT;
		m_previousMousePos = zoomedMouse;
		return true;
	}

	if((zoomedMouse - m_endPos).manhattanLength() < 10)
	{
		m_movingAction = MOVING_POINT;
		m_previousMousePos = zoomedMouse;
		return true;
	}

	return false;
}

void AnnotationDrawStruct::mouseMoveEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / m_parentView->getZoom();
	QPointF delta = zoomedMouse - m_previousMousePos;
	m_previousMousePos = zoomedMouse;
	if(delta.isNull())
		return;

	if(m_movingAction == MOVING_TEXT)
		m_parentView->getDocument()->addCommand(new MoveAnnotationTextCommand(this, delta));
	else if(m_movingAction == MOVING_POINT)
		m_parentView->getDocument()->addCommand(new MoveAnnotationEndCommand(this, delta));
}

void AnnotationDrawStruct::mouseReleaseEvent(QMouseEvent*)
{
	m_movingAction = MOVING_NONE;
}

QSize AnnotationDrawStruct::getObjectSize()
{
	return QSize(100, 50);
}

int AnnotationDrawClass = RegisterDrawObject<panda::Annotation, AnnotationDrawStruct>();
