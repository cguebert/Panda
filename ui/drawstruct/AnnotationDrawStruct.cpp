#include <ui/graphview/GraphView.h>
#include <ui/drawstruct/AnnotationDrawStruct.h>
#include <ui/command/ModifyAnnotationCommand.h>
#include <ui/graphview/ObjectsSelection.h>

#include <panda/object/Annotation.h>
#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>

#include <QMouseEvent>

using panda::Annotation;
using panda::types::Point;
using panda::types::Rect;

namespace
{
	inline Point convert(const QPointF& pt)
	{
		return Point(pt.x(), pt.y());
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
	if(m_textSize.isNull() || m_textCounter != textCounter || m_colorCounter != colorCounter)
	{
		auto tempSize = list.calcTextSize(1.0f, m_annotation->m_text.getValue());
		m_textSize = Point(tempSize.x + 1, tempSize.y);
		m_textCounter = textCounter;
		m_colorCounter = colorCounter;
		update();
	}

	// Draw the shape of the annotation
	auto fillColor = DrawList::convert(m_annotation->m_color.getValue().toHex());
	list.addMesh(m_fillShape, fillColor);
	list.addPolyline(m_outline, colors.penColor, false);
}

void AnnotationDrawStruct::drawForeground(DrawList& list, DrawColors& colors)
{
	auto textArea = pRect(m_textArea.left(), m_textArea.top(), m_textArea.right(), m_textArea.bottom());

	// Draw the box behind the text
	list.addRectFilled(textArea, colors.midLightColor);

	// Draw the text
	textArea.adjust(5, 5, -5, -5);
	list.addText(textArea, m_annotation->m_text.getValue(), colors.penColor);

	// Draw the handle
	const panda::PandaDocument* doc = getParentView()->getDocument();
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& getParentView()->selection().get().size() == 1
			&& getParentView()->selection().isSelected(m_annotation))	// The annotation is the only selected object
	{
		list.addCircleFilled(m_endPos, 5, colors.midLightColor);
		list.addCircle(m_endPos, 5, colors.penColor);
	}
}

void AnnotationDrawStruct::move(const Point& delta)
{
	ObjectDrawStruct::move(delta);

	m_textArea.translate(delta);
	m_endPos += delta;
}

void AnnotationDrawStruct::moveText(const Point& delta, bool emitModified)
{
	move(delta);
	if (emitModified)
		emit getParentView()->modified();
	getParentView()->update();
}

void AnnotationDrawStruct::moveEnd(const Point& delta, bool emitModified)
{
	m_endPos += delta;
	update();
	if (emitModified)
	{
		m_annotation->m_deltaToEnd.setValue(m_annotation->m_deltaToEnd.getValue() + delta);
		emit getParentView()->modified();
	}
	getParentView()->update();
}

bool AnnotationDrawStruct::contains(const Point& point)
{
	const panda::PandaDocument* doc = getParentView()->getDocument();
	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT
			&& getParentView()->selection().get().size() == 1
			&& getParentView()->selection().isSelected(m_annotation))	// The annotation is the only selected object
	{
		if((point - m_endPos).norm() < 10)
			return true;
	}

	return m_textArea.contains(point);
}

void AnnotationDrawStruct::update()
{
//	ObjectDrawStruct::update();	// No need to call it

	if (m_movingAction == MOVING_NONE)
		m_endPos = getPosition() + m_annotation->m_deltaToEnd.getValue();

	m_textArea = Rect::fromSize(getPosition(), m_textSize);
	m_textArea.translate(0, -m_textSize.y);
	m_textArea.adjust(3, -13, 13, -3);

	m_annotation->cleanDirty();
	m_selectionArea = m_visualArea = m_textArea;

	if(m_annotation->m_type.getValue() != Annotation::ANNOTATION_TEXT)
		 m_visualArea |= Rect(getPosition(), m_endPos).canonicalized();

	createShape();
}

void AnnotationDrawStruct::createShape()
{
	m_outline.clear();
	switch(m_annotation->m_type.getValue())
	{
		case Annotation::ANNOTATION_TEXT:
			break;
		case Annotation::ANNOTATION_ARROW:
		{
			Point start = m_textArea.center();
			Point dir = m_endPos - start;
			const float w = 2.5;
			float length = dir.norm();

			if(length < w * 20)
				break;

			dir /= length;
			Point dir2 = Point(-dir.y, dir.x);
			dir *= w;
			dir2 *= w;

			m_outline.moveTo(m_endPos);
			m_outline.lineTo(m_endPos + 5*dir2 - 15*dir);
			m_outline.lineTo(m_endPos + dir2 - 10*dir);
			m_outline.lineTo(start + dir2);
			m_outline.lineTo(start - dir2);
			m_outline.lineTo(m_endPos - dir2 - 10*dir);
			m_outline.lineTo(m_endPos - 5*dir2 - 15*dir);
			m_outline.lineTo(m_endPos);
			break;
		}
		case Annotation::ANNOTATION_RECTANGLE:
		{
			m_outline.rect(Rect(getPosition(), m_endPos));
			m_outline.close();
			break;
		}
		case Annotation::ANNOTATION_ELLIPSE:
		{
			m_outline.arcToDegrees(Rect(getPosition(), m_endPos), 0, 360);
			break;
		}
	}

	m_fillShape = m_outline.triangulate();
}

bool AnnotationDrawStruct::mousePressEvent(QMouseEvent* event)
{
	Point zoomedMouse = getParentView()->getViewDelta() + convert(event->localPos() )/ getParentView()->getZoom();

	if(m_textArea.contains(zoomedMouse))
	{
		m_movingAction = MOVING_TEXT;
		m_startMousePos = m_previousMousePos = zoomedMouse;
		return true;
	}

	if((zoomedMouse - m_endPos).norm() < 10)
	{
		m_movingAction = MOVING_POINT;
		m_startMousePos = m_previousMousePos = zoomedMouse;
		return true;
	}

	return false;
}

void AnnotationDrawStruct::mouseMoveEvent(QMouseEvent* event)
{
	Point zoomedMouse = getParentView()->getViewDelta() + convert(event->localPos()) / getParentView()->getZoom();
	Point delta = zoomedMouse - m_previousMousePos;
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
	Point zoomedMouse = getParentView()->getViewDelta() + convert(event->localPos()) / getParentView()->getZoom();
	Point deltaStart = m_startMousePos - m_previousMousePos;
	Point delta = zoomedMouse - m_startMousePos;

	if (m_movingAction == MOVING_TEXT)
	{
		moveText(deltaStart, false);
		getParentView()->getDocument()->getUndoStack().push(std::make_shared<MoveAnnotationTextCommand>(this, delta));
	}
	else if (m_movingAction == MOVING_POINT)
	{
		moveEnd(deltaStart, false);
		getParentView()->getDocument()->getUndoStack().push(std::make_shared<MoveAnnotationEndCommand>(this, delta));
	}
		
	m_movingAction = MOVING_NONE;
}

Point AnnotationDrawStruct::getObjectSize()
{
	return Point(100, 50);
}

int AnnotationDrawClass = RegisterDrawObject<panda::Annotation, AnnotationDrawStruct>();
