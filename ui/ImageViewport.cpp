#include <QtWidgets>
#include <ui/ImageViewport.h>

#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/types/ImageWrapper.h>

#include <iostream>

using panda::Data;
using panda::types::ImageWrapper;
typedef Data<ImageWrapper> ImageData;

ImageViewport::ImageViewport(const panda::BaseData* data, QGLWidget* shareWidget, QWidget* parent)
	: QGLWidget(parent, shareWidget)
	, m_data(data)
	, m_zoomLevel(0)
	, m_wheelTicks(0)
	, m_zoomFactor(1.0)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	addInput(const_cast<panda::BaseData*>(data));
}

QSize ImageViewport::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize ImageViewport::sizeHint() const
{
/*	const ImageData* imageData = dynamic_cast<const ImageData*>(m_data);
	const ImageWrapper& img = imageData->getValue();
	if(img.isNull())
		return QSize(600, 400);
	return img.size();*/
	return QSize(600, 400);
}

void ImageViewport::setDirtyValue(const panda::DataNode* /*caller*/)
{
	if(!m_data->isDirty() // Just got modified
	|| !m_data->getOwner()->getParentDocument()->animationIsPlaying()) // Or animation not playing
		QWidget::update();
}

void ImageViewport::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);
	if(node == m_data)
		emit closeViewport(this);
}

void ImageViewport::initializeGL()
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
}

void ImageViewport::paintGL()
{
	const ImageData* imageData = dynamic_cast<const ImageData*>(m_data);
	const ImageWrapper& img = imageData->getValue();

	QColor col = palette().window().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glDepthMask( GL_TRUE );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(img.isNull())
	{
		resize(QSize(320, 240));
		return;
	}

	QSize imgSize = img.size();
	QSize renderSize = imgSize * m_zoomFactor;
	QRect viewRect = contentsRect();

	if(renderSize != viewRect.size())
		resize(renderSize);

	glViewport(0, 0, renderSize.width(), renderSize.height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, imgSize.width(), imgSize.height(), 0, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(1, 1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	drawTexture(QPointF(), img.getTextureId());

	glDisable(GL_BLEND);
}

void ImageViewport::wheelEvent(QWheelEvent* event)
{
	m_wheelTicks += event->angleDelta().y();
	int ticks = m_wheelTicks / 40; // Steps of 5 degrees
	m_wheelTicks -= ticks * 40;
	int newZoom = qBound(0, m_zoomLevel - ticks, 90);
	if(m_zoomLevel != newZoom)
	{
		m_zoomLevel = newZoom;
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		QWidget::update();
	}
}
