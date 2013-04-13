#include <QtWidgets>

#include <ui/RenderView.h>
#include <panda/PandaDocument.h>

RenderView::RenderView(panda::PandaDocument* doc, QWidget *parent)
	: QWidget(parent)
	, pandaDocument(doc)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setMouseTracking(true);
}

QSize RenderView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize RenderView::sizeHint() const
{
	return QSize(600, 400);
}

void RenderView::paintEvent(QPaintEvent* /*event*/)
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

void RenderView::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		pandaDocument->setMouseClick(1);
}

void RenderView::mouseMoveEvent(QMouseEvent *event)
{
	QRect viewRect = contentsRect();
	QSize renderSize = pandaDocument->getRenderSize();
	QPointF delta = QPointF(viewRect.center().x() - renderSize.width() / 2,
							viewRect.center().y() - renderSize.height() / 2);

	pandaDocument->setMousePosition(event->localPos() - delta);
}

void RenderView:: mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		pandaDocument->setMouseClick(0);
}
