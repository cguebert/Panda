#include <QtGui>

#include <ui/RenderView.h>
#include <panda/PandaDocument.h>

RenderView::RenderView(panda::PandaDocument* doc, QWidget *parent)
	: QWidget(parent)
	, pandaDocument(doc)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
