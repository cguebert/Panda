#include <QtWidgets>
#include <QOpenGLFramebufferObject>

#include <ui/OpenGLRenderView.h>
#include <panda/PandaDocument.h>

#include <iostream>

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent)
	: QGLWidget(parent)
	, pandaDocument(doc)
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

void OpenGLRenderView::initializeGL()
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
}

void OpenGLRenderView::paintGL()
{
	QColor col = palette().window().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glDepthMask( GL_TRUE );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QOpenGLFramebufferObject* fbo = pandaDocument->getFBO().data();

	QRect viewRect = contentsRect();
	glViewport(0, 0, viewRect.width(), viewRect.height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewRect.width(), 0, viewRect.height(), -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	QSize renderSize = fbo->size();
	glColor4f(1, 1, 1, 1);
	drawTexture(QPointF(viewRect.center().x() - renderSize.width() / 2,
						viewRect.center().y() - renderSize.height() / 2), fbo->texture());
}

void OpenGLRenderView::resizeGL(int /*width*/, int /*height*/)
{
}

void OpenGLRenderView::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		pandaDocument->setMouseClick(1);
}

void OpenGLRenderView::mouseMoveEvent(QMouseEvent *event)
{
	QRect viewRect = contentsRect();
	QSize renderSize = pandaDocument->getRenderSize();
	QPointF delta = QPointF(viewRect.center().x() - renderSize.width() / 2,
							viewRect.center().y() - renderSize.height() / 2);

	pandaDocument->setMousePosition(event->localPos() - delta);
}

void OpenGLRenderView:: mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		pandaDocument->setMouseClick(0);
}
