#include <QtWidgets>

#include <ui/OpenGLRenderView.h>

#include <panda/PandaDocument.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Mat4x4.h>
#include <panda/graphics/ShaderProgram.h>

namespace
{
	QSize convert(panda::graphics::Size size)
	{ return QSize(size.width(), size.height()); }

	panda::graphics::Size convert(QSize size)
	{ return panda::graphics::Size(size.width(), size.height()); }
}

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent)
	: QOpenGLWidget(parent)
	, m_document(doc)
	, m_adjustRenderSize(false)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setMouseTracking(true);

	resize(convert(doc->getRenderSize()));

	m_observer.get(doc->m_renderSizeChangedSignal).connect<OpenGLRenderView, &OpenGLRenderView::renderSizeChanged>(this);
}

QSize OpenGLRenderView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize OpenGLRenderView::sizeHint() const
{
	return convert(m_document->getRenderSize());
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

void OpenGLRenderView::renderSizeChanged()
{
	if(!m_adjustRenderSize)
		resize(convert(m_document->getRenderSize()));
}

void OpenGLRenderView::initializeGL()
{
	m_document->getRenderer()->initializeGL();
}

void OpenGLRenderView::resizeGL(int w, int h)
{
	m_document->getRenderer()->resizeGL(w, h);
}

void OpenGLRenderView::paintGL()
{
	m_document->getRenderer()->setRenderingMainView(true);
	m_document->updateIfDirty();
	m_document->getRenderer()->setRenderingMainView(false);
	auto fbo = m_document->getFBO();

	QRect viewRect = contentsRect();
	panda::graphics::RectInt rect(0, 0, viewRect.width(), viewRect.height());
	panda::graphics::Framebuffer::blitFramebuffer(defaultFramebufferObject(), rect, fbo.id(), rect);
}

void OpenGLRenderView::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
		m_document->setMouseClick(true, panda::types::Point(event->localPos().x(), event->localPos().y()));
}

void OpenGLRenderView::mouseMoveEvent(QMouseEvent* event)
{
	m_document->setMousePosition(panda::types::Point(event->localPos().x(), event->localPos().y()));
}

void OpenGLRenderView::mouseReleaseEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
		m_document->setMouseClick(false, panda::types::Point(event->localPos().x(), event->localPos().y()));
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
