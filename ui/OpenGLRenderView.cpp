#include <QWindow>
#include <QWidget>
#include <QMouseEvent>
#include <QCoreApplication>

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

#include <ui/OpenGLRenderView.h>
#include <panda/PandaDocument.h>

#include <iostream>

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* document, QWindow* parent)
	: QWindow(parent)
	, m_document(document)
	, m_containingWidget(nullptr)
	, m_context(nullptr)
	, m_adjustRenderSize(false)
	, m_updatePending(false)
{
	setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLRenderView::~OpenGLRenderView()
{
}

void OpenGLRenderView::createOpenGLRenderView(panda::PandaDocument* document, OpenGLRenderView*& view, QWidget*& container)
{
	view = new OpenGLRenderView(document);
	container = QWidget::createWindowContainer(view);
	view->setContainer(container);

	container->setAutoFillBackground(true);
	container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	container->setMouseTracking(true);

	container->setMinimumSize(300, 200);
}

void OpenGLRenderView::setContainer(QWidget* container)
{
	m_containingWidget = container;
}

void OpenGLRenderView::update()
{
	if (!m_updatePending)
	{
		m_updatePending = true;
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
	}
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
		QRect viewRect = m_containingWidget->contentsRect();
		m_document->setRenderSize(viewRect.size());
	}
}

void OpenGLRenderView::render()
{
	if (!m_context)
	{
		m_context = new QOpenGLContext(this);
		m_context->setFormat(QSurfaceFormat());
		m_context->create();

		m_context->makeCurrent(this);

		m_shaderProgram = new QOpenGLShaderProgram(this);
		m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/PT_noColor_Tex.v.glsl");
		m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/PT_noColor_Tex.f.glsl");
		m_shaderProgram->link();
	}

	m_context->makeCurrent(this);

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	QOpenGLFramebufferObject* fbo = m_document->getFBO().data();

	QColor col = m_containingWidget->palette().window().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glDepthMask( GL_TRUE );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QRect viewRect = m_containingWidget->contentsRect();
	glViewport(0, 0, viewRect.width(), viewRect.height());

	QSize renderSize = fbo->size();
	int left = (viewRect.width() - renderSize.width()) / 2;
	int top = (viewRect.height() - renderSize.height()) / 2;
	int right = left + renderSize.width();
	int bottom = top + renderSize.height();

	GLfloat verts[8], texCoords[8];
	texCoords[0*2+0] = 1; texCoords[0*2+1] = 1;
	texCoords[1*2+0] = 0; texCoords[1*2+1] = 1;
	texCoords[3*2+0] = 0; texCoords[3*2+1] = 0;
	texCoords[2*2+0] = 1; texCoords[2*2+1] = 0;

	verts[0*2+0] = right; verts[0*2+1] = top;
	verts[1*2+0] = left;  verts[1*2+1] = top;
	verts[3*2+0] = left;  verts[3*2+1] = bottom;
	verts[2*2+0] = right; verts[2*2+1] = bottom;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_shaderProgram->bind();
	QMatrix4x4 MVP;
	MVP.ortho(0, viewRect.width(), viewRect.height(), 0, -10, 10);
	m_shaderProgram->setUniformValue("MVP", MVP);

	m_shaderProgram->enableAttributeArray("vertex");
	m_shaderProgram->setAttributeArray("vertex", verts, 2);

	m_shaderProgram->enableAttributeArray("texCoord");
	m_shaderProgram->setAttributeArray("texCoord", texCoords, 2);

	glBindTexture(GL_TEXTURE_2D, fbo->texture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_shaderProgram->setUniformValue("tex0", 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_shaderProgram->disableAttributeArray("vertex");
	m_shaderProgram->disableAttributeArray("texCoord");
	m_shaderProgram->release();

	glDisable(GL_BLEND);

	m_context->swapBuffers(this);
	m_context->makeCurrent(this);
}

void OpenGLRenderView::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
		m_document->setMouseClick(1);
}

void OpenGLRenderView::mouseMoveEvent(QMouseEvent* event)
{
	QRect viewRect = m_containingWidget->contentsRect();
	QSize renderSize = m_document->getRenderSize();
	QPointF pos = event->localPos() - QPointF(viewRect.center().x() - renderSize.width() / 2,
							viewRect.center().y() - renderSize.height() / 2);

	m_document->setMousePosition(panda::types::Point(pos.x(), pos.y()));
}

void OpenGLRenderView::mouseReleaseEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
		m_document->setMouseClick(0);
}

bool OpenGLRenderView::event(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::UpdateRequest:
		m_updatePending = false;
		render();
		return true;
	default:
		return QWindow::event(event);
	}
}

void OpenGLRenderView::resizeEvent(QResizeEvent*)
{
	if(m_adjustRenderSize)
	{
		QRect viewRect = m_containingWidget->contentsRect();
		m_document->setRenderSize(viewRect.size());
	}
	update();
}
