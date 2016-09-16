#include <QtWidgets>

#include <ui/OpenGLRenderView.h>

#include <panda/document/InteractiveDocument.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/document/DocumentSignals.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Mat4x4.h>
#include <panda/graphics/ShaderProgram.h>

namespace
{
	QSize convert(panda::graphics::Size size)
	{ return QSize(size.width(), size.height()); }

	panda::graphics::Size convert(QSize size)
	{ return panda::graphics::Size(size.width(), size.height()); }

	int convertKey(int key)
	{
		if (key > 0 && key < 0xff)
			return key;

		// Convert to the glfw values
		switch (key)
		{
		case Qt::Key_Tab: return 258;
		case Qt::Key_Left: return 263;
		case Qt::Key_Right: return 262;
		case Qt::Key_Up: return 265;
		case Qt::Key_Down: return 264;
		case Qt::Key_PageUp: return 266;
		case Qt::Key_PageDown: return 267;
		case Qt::Key_Home: return 268;
		case Qt::Key_End: return 269;
		case Qt::Key_Delete: return 261;
		case Qt::Key_Backspace: return 259;
		case Qt::Key_Enter: return 257;
		case Qt::Key_Escape: return 256;
		case Qt::Key_Shift: return 340;
		case Qt::Key_Control: return 341;
		case Qt::Key_Alt: return 342;
		default: return -1;
		}
	}
}

OpenGLRenderView::OpenGLRenderView(panda::InteractiveDocument* doc, QWidget *parent)
	: QOpenGLWidget(parent)
	, m_document(doc)
	, m_adjustRenderSize(false)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	setMouseTracking(true);
}

QSize OpenGLRenderView::minimumSizeHint() const
{
	return QSize(300, 200);
}

void OpenGLRenderView::setAdjustRenderSize(bool adjust)
{
	m_adjustRenderSize = adjust;
	if(m_adjustRenderSize)
	{
		QRect viewRect = contentsRect();
		m_document->setRenderSize(convert(viewRect.size()));
	}
}

void OpenGLRenderView::initializeGL()
{
	m_document->getRenderer().initializeGL();
}

void OpenGLRenderView::resizeGL(int w, int h)
{
	if (m_adjustRenderSize)
		m_document->setRenderSize({ w, h });
}

void OpenGLRenderView::paintGL()
{
	m_document->getRenderer().setRenderingMainView(true);
	m_document->updateIfDirty();
	m_document->getRenderer().setRenderingMainView(false);
	auto fbo = m_document->getFBO();

	QRect viewRect = contentsRect();
	auto col = palette().window().color();
	glViewport(0, 0, viewRect.width(), viewRect.height());
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto docSize = fbo.size();
	int w = docSize.width(), h = docSize.height();
	panda::graphics::RectInt srcRect(0, 0, w, h);

	m_deltaPos = { (viewRect.width() - w) / 2, (viewRect.height() - h) / 2 };
	panda::graphics::RectInt dstRect(m_deltaPos.x, m_deltaPos.y, m_deltaPos.x + w, m_deltaPos.y + h);
	panda::graphics::Framebuffer::blitFramebuffer(defaultFramebufferObject(), dstRect, fbo.id(), srcRect);

	m_document->getSignals().postRender.run(viewRect.width(), viewRect.height(), defaultFramebufferObject());
}

void OpenGLRenderView::mouseMoveEvent(QMouseEvent* event)
{
	panda::types::Point globalPos(event->localPos().x(), event->localPos().y());
	panda::types::Point localPos(globalPos.x - m_deltaPos.x, globalPos.y - m_deltaPos.y);
	m_document->mouseMoveEvent(localPos, globalPos);
}

void OpenGLRenderView::mousePressEvent(QMouseEvent* event)
{
	panda::types::Point pos(event->localPos().x() - m_deltaPos.x, event->localPos().y() - m_deltaPos.y);
	switch (event->button())
	{
	case Qt::LeftButton:	m_document->mouseButtonEvent(0, true, pos); break;
	case Qt::RightButton:	m_document->mouseButtonEvent(1, true, pos); break;
	case Qt::MiddleButton:	m_document->mouseButtonEvent(2, true, pos); break;
	}
}

void OpenGLRenderView::mouseReleaseEvent(QMouseEvent* event)
{
	panda::types::Point pos(event->localPos().x() - m_deltaPos.x, event->localPos().y() - m_deltaPos.y);
	switch (event->button())
	{
	case Qt::LeftButton:	m_document->mouseButtonEvent(0, false, pos); break;
	case Qt::RightButton:	m_document->mouseButtonEvent(1, false, pos); break;
	case Qt::MiddleButton:	m_document->mouseButtonEvent(2, false, pos); break;
	}
}

void OpenGLRenderView::keyPressEvent(QKeyEvent* event)
{
	int key = convertKey(event->key());
	if (key != -1)
		m_document->keyEvent(key, true);

	auto text = event->text();
	if (!text.isEmpty())
		m_document->textEvent(text.toStdString());
}

void OpenGLRenderView::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	auto mods = event->modifiers();
	if (key == Qt::Key_Control && mods & Qt::ControlModifier)
		return; // The other key is still pressed
	if (key == Qt::Key_Alt && mods & Qt::AltModifier)
		return; // The other key is still pressed

	key = convertKey(key);
	if (key != -1)
		m_document->keyEvent(key, false);
}

void OpenGLRenderView::resizeEvent(QResizeEvent* event)
{
	QOpenGLWidget::resizeEvent(event);
	if(m_adjustRenderSize)
	{
		QRect viewRect = contentsRect();
		m_document->setRenderSize(convert(viewRect.size()));
	}
}

void OpenGLRenderView::focusOutEvent(QFocusEvent*)
{
	emit lostFocus(this);
}
