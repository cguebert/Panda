#include <ui/graphview/GraphView.h>
#include <ui/drawstruct/AnnotationDrawStruct.h>
#include <ui/command/ModifyAnnotationCommand.h>
#include <ui/graphview/ObjectsSelection.h>

#include <panda/object/Annotation.h>
#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>

#include <QPainter>
#include <QMouseEvent>

using panda::Annotation;
using panda::types::Point;

namespace
{
	inline pPoint convert(const QPointF& pt)
	{
		return pPoint(pt.x(), pt.y());
	}
}

AnnotationDrawStruct::AnnotationDrawStruct(GraphView* view, panda::PandaObject* object)
	: ObjectDrawStruct(view, object)
	, m_annotation(dynamic_cast<Annotation*>(object))
{
	update();
}

void AnnotationDrawStruct::drawBackground(DrawList& list, DrawColors& colors)
{
	// Compute the bounding box of the text, if it changed
	int textCounter = m_annotation->m_text.getCounter();
	int colorCounter = m_annotation->m_color.getCounter();
	if(m_textSize.isEmpty() || m_textCounter != textCounter || m_colorCounter != colorCounter)
	{
		auto tempSize = list.calcTextSize(1.0f, m_annotation->m_text.getValue());
		m_textSize = QSizeF(tempSize.x + 1, tempSize.y);
		m_textCounter = textCounter;
		m_colorCounter = colorCounter;
		update();
	}

	// Draw the shape of the annotation
	list.addConvexPolyFilled(m_shapePath, DrawList::convert(m_annotation->m_color.getValue().toHex()));
	list.addPolyline(m_shapePath, colors.penColor, false);
}

void AnnotationDrawStruct::drawForeground(DrawList& list, DrawColors& colors)
{
	auto textArea = pRect(m_textArea.left(), m_textArea.top(), m_textArea.right(), m_textArea.bottom());

	// Draw the box behind the text
	list.addRectFilled(textArea.topLeft(), textArea.bottomRight(), colors.midLightColor);

	// Draw the text
	textArea.adjust(5, 5, -5, -5);
	list.addText(textArea, m_annotation->m_text.getValue(), colors.penColor);

	// Draw the handle
	const panda::PandaDocument* doc = m_parentView->getDocument();
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& m_parentView->selection().get().size() == 1
			&& m_parentView->selection().isSelected(m_annotation))	// The annotation is the only selected object
	{
		auto center = pPoint(m_endPos.x(), m_endPos.y());
		list.addCircleFilled(center, 5, colors.midLightColor);
		list.addCircle(center, 5, colors.penColor);
	}
}

void AnnotationDrawStruct::moveVisual(const QPointF& delta)
{
	ObjectDrawStruct::moveVisual(delta);
	auto pDelta = pPoint(delta.x(), delta.y());
	m_shapePath.translate(pDelta);
	m_shapeMesh.translate(pDelta);

	m_textArea.translate(delta);
	m_startPos += delta;
	m_endPos += delta;
}

void AnnotationDrawStruct::moveText(const QPointF& delta, bool emitModified)
{
	move(delta);
	if (emitModified)
		emit m_parentView->modified();
	m_parentView->update();
}

void AnnotationDrawStruct::moveEnd(const QPointF& delta, bool emitModified)
{
	m_endPos += delta;
	update();
	if (emitModified)
	{
		m_annotation->m_deltaToEnd.setValue(m_annotation->m_deltaToEnd.getValue() + Point(delta.x(), delta.y()));
		emit m_parentView->modified();
	}
	m_parentView->update();
}

bool AnnotationDrawStruct::contains(const QPointF& point)
{
	const panda::PandaDocument* doc = m_parentView->getDocument();
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& m_parentView->selection().get().size() == 1
			&& m_parentView->selection().isSelected(m_annotation))	// The annotation is the only selected object
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
	auto deltaToEnd = m_annotation->m_deltaToEnd.getValue();
	if(m_movingAction == MOVING_NONE)
		m_endPos = m_startPos + QPointF(deltaToEnd.x, deltaToEnd.y);

	m_textArea = QRectF(m_startPos, m_textSize);
	m_textArea.translate(0, -m_textSize.height());
	m_textArea.adjust(3, -13, 13, -3);

	m_shapePath.clear();
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

			m_shapePath.moveTo(convert(m_endPos));
			m_shapePath.lineTo(convert(m_endPos + 5*dir2 - 15*dir));
			m_shapePath.lineTo(convert(m_endPos + dir2 - 10*dir));
			m_shapePath.lineTo(convert(start + dir2));
			m_shapePath.lineTo(convert(start - dir2));
			m_shapePath.lineTo(convert(m_endPos - dir2 - 10*dir));
			m_shapePath.lineTo(convert(m_endPos - 5*dir2 - 15*dir));
			m_shapePath.lineTo(convert(m_endPos));
			break;
		}
		case Annotation::ANNOTATION_RECTANGLE:
		{
			m_shapePath.rect(pPoint(m_startPos.x(), m_startPos.y()), pPoint(m_endPos.x(), m_endPos.y()));
			m_shapePath.close();
			break;
		}
		case Annotation::ANNOTATION_ELLIPSE:
		{
			m_shapePath.arcToDegrees(pRect(m_startPos.x(), m_startPos.y(), m_endPos.x(), m_endPos.y()), 0, 360);
			break;
		}
	}

	m_shapeMesh = m_shapePath.triangulate();

	m_annotation->cleanDirty();
	m_objectArea = m_textArea;
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT)
		 m_objectArea |= QRectF(m_startPos, m_endPos).normalized();
}

bool AnnotationDrawStruct::mousePressEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / m_parentView->getZoom();

	if(m_textArea.contains(zoomedMouse))
	{
		m_movingAction = MOVING_TEXT;
		m_startMousePos = m_previousMousePos = zoomedMouse;
		return true;
	}

	if((zoomedMouse - m_endPos).manhattanLength() < 10)
	{
		m_movingAction = MOVING_POINT;
		m_startMousePos = m_previousMousePos = zoomedMouse;
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

	if (m_movingAction == MOVING_TEXT)
		moveText(delta, false);
	else if (m_movingAction == MOVING_POINT)
		moveEnd(delta, false);
}

void AnnotationDrawStruct::mouseReleaseEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / m_parentView->getZoom();
	QPointF deltaStart = m_startMousePos - m_previousMousePos;
	QPointF delta = zoomedMouse - m_startMousePos;

	if (m_movingAction == MOVING_TEXT)
	{
		moveText(deltaStart, false);
		m_parentView->getDocument()->getUndoStack().push(std::make_shared<MoveAnnotationTextCommand>(this, delta));
	}
	else if (m_movingAction == MOVING_POINT)
	{
		moveEnd(deltaStart, false);
		m_parentView->getDocument()->getUndoStack().push(std::make_shared<MoveAnnotationEndCommand>(this, delta));
	}
		
	m_movingAction = MOVING_NONE;
}

QSize AnnotationDrawStruct::getObjectSize()
{
	return QSize(100, 50);
}

int AnnotationDrawClass = RegisterDrawObject<panda::Annotation, AnnotationDrawStruct>();
