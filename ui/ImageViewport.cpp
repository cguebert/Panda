#include <QtWidgets>
#include <ui/ImageViewport.h>

#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/types/ImageWrapper.h>

#include <iostream>

using panda::Data;
using panda::types::ImageWrapper;
using ImageData = Data<ImageWrapper>;

ImageViewport::ImageViewport(const panda::BaseData* data, QWidget* parent)
	: QOpenGLWidget(parent)
	, m_data(data)
	, m_zoomLevel(0)
	, m_wheelTicks(0)
	, m_zoomFactor(1.0)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	addInput(*const_cast<panda::BaseData*>(data));
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

void ImageViewport::doRemoveInput(DataNode& node)
{
	DataNode::doRemoveInput(node);
	if(&node == m_data)
		emit closeViewport(this);
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

	auto imgSize = img.size();
	auto renderSize = imgSize * m_zoomFactor;
	QSize qRenderSize(renderSize.width(), renderSize.height());
	QRect viewRect = contentsRect();

	if(qRenderSize != viewRect.size())
		resize(qRenderSize);

	glViewport(0, 0, viewRect.width(), viewRect.height());
	auto fbo = img.getFbo();
	if (fbo)
	{
		panda::graphics::RectInt rect(0, 0, img.width(), img.height());
		panda::graphics::Framebuffer::blitFramebuffer(defaultFramebufferObject(), rect, fbo->id(), rect);
	}
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
