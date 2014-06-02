#include <QtWidgets>
#include <QOpenGLFramebufferObject>

#include <ui/OpenGLRenderView.h>
#include <panda/PandaDocument.h>

#include <iostream>

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent)
	: QGLWidget(parent)
	, m_document(doc)
	, m_adjustRenderSize(false)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setMouseTracking(true);
}

OpenGLRenderView::~OpenGLRenderView()
{
}

QSize OpenGLRenderView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize OpenGLRenderView::sizeHint() const
{
	return QSize(600, 400);
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

void OpenGLRenderView::initializeGL()
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
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

	drawTexture(QPointF(viewRect.center().x() - renderSize.width() / 2,
						viewRect.center().y() - renderSize.height() / 2), fbo->texture());

	glDisable(GL_BLEND);
}

void OpenGLRenderView::resizeGL(int /*width*/, int /*height*/)
{
}

void OpenGLRenderView::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		m_document->setMouseClick(1);
}

void OpenGLRenderView::mouseMoveEvent(QMouseEvent *event)
{
	QRect viewRect = contentsRect();
	QSize renderSize = m_document->getRenderSize();
	QPointF pos = event->localPos() - QPointF(viewRect.center().x() - renderSize.width() / 2,
							viewRect.center().y() - renderSize.height() / 2);

	m_document->setMousePosition(panda::types::Point(pos.x(), pos.y()));
}

void OpenGLRenderView::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		m_document->setMouseClick(0);
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
