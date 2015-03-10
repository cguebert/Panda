#include <QtWidgets>
#include <QOpenGLFramebufferObject>

#include <ui/OpenGLRenderView.h>
#include <panda/PandaDocument.h>

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent)
	: QGLWidget(parent)
	, m_document(doc)
	, m_adjustRenderSize(false)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setMouseTracking(true);

	resize(doc->getRenderSize());

	connect(doc, SIGNAL(renderSizeChanged()), this, SLOT(renderSizeChanged()));
}

QSize OpenGLRenderView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize OpenGLRenderView::sizeHint() const
{
	return m_document->getRenderSize();
}

void OpenGLRenderView::setAdjustRenderSize(bool adjust)
{
	m_adjustRenderSize = adjust;
	if(m_adjustRenderSize)
	{
		QRect viewRect = contentsRect();
		m_document->setRenderSize(viewRect.size());
	}
}

void OpenGLRenderView::renderSizeChanged()
{
	if(!m_adjustRenderSize)
		resize(m_document->getRenderSize());
}

void OpenGLRenderView::initializeGL()
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	m_document->initializeGL();
}

void OpenGLRenderView::paintGL()
{
	QOpenGLFramebufferObject* fbo = m_document->getFBO().data();

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

	QSize renderSize = fbo->size();
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
		m_document->setRenderSize(viewRect.size());
	}
}
