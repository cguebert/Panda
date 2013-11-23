#include <QtWidgets>

#include <ui/OpenGLRenderView.h>
#include <panda/PandaDocument.h>

OpenGLRenderView::OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent)
	: QGLWidget(parent)
	, pandaDocument(doc)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setMouseTracking(true);
}

QSize OpenGLRenderView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize OpenGLRenderView::sizeHint() const
{
	return QSize(600, 400);
}

void OpenGLRenderView::paintEvent(QPaintEvent* /*event*/)
{
	QStylePainter painter(this);

	QImage image = pandaDocument->getRenderedImage();
	if(!image.isNull())
	{
		QRect viewRect = contentsRect();
		painter.drawImage(viewRect.center().x() - image.width() / 2,
						   viewRect.center().y() - image.height() / 2,
						   image);
	}
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
