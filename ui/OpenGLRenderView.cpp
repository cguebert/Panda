#include <QtWidgets>

#include <ui/OpenGLRenderView.h>

#include <panda/PandaDocument.h>
#include <panda/graphics/Framebuffer.h>

namespace
{
	QSize convert(panda::graphics::Size size)
	{ return QSize(size.width(), size.height()); }

	panda::graphics::Size convert(QSize size)
	{ return panda::graphics::Size(size.width(), size.height()); }
}

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent)
	: QGLWidget(parent)
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
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	m_document->initializeGL();
}

void OpenGLRenderView::paintGL()
{
	auto* fbo = m_document->getFBO().get();

	QColor col = palette().window().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glDepthMask( GL_TRUE );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QRect viewRect = contentsRect();
	glViewport(0, 0, viewRect.width(), viewRect.height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewRect.width(), viewRect.height(), 0, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(1, 1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	drawTexture(QPointF(), fbo->texture());

	glDisable(GL_BLEND);
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
	QGLWidget::resizeEvent(event);
	if(m_adjustRenderSize)
	{
		QRect viewRect = contentsRect();
		m_document->setRenderSize(convert(viewRect.size()));
	}
}
