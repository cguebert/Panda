#include <QtWidgets>
#include <ui/ImageViewport.h>

#include <panda/types/ImageWrapper.h>

#include <iostream>

using panda::Data;
using panda::types::ImageWrapper;
typedef Data<ImageWrapper> ImageData;

ImageViewport::ImageViewport(const panda::BaseData* data, QGLWidget* shareWidget, QWidget* parent)
	: QGLWidget(parent, shareWidget)
	, m_data(data)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	addInput(const_cast<panda::BaseData*>(data));
}

ImageViewport::~ImageViewport()
{
//	std::cout << "~ImageViewport()" << std::endl;
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

void ImageViewport::setDirtyValue(const panda::DataNode* caller)
{
	if(caller == m_data && !m_data->isDirty()) // Just got modified
		QWidget::update();
}

void ImageViewport::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);
	if(node == m_data)
	{
		// TODO : close this viewport
//		std::cout << "close viewport" << std::endl;
	}
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
		return;

	QRect viewRect = contentsRect();
	glViewport(0, 0, viewRect.width(), viewRect.height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewRect.width(), viewRect.height(), 0, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	QSize renderSize = img.size();
	glColor4f(1, 1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	drawTexture(QPointF(viewRect.center().x() - renderSize.width() / 2,
						viewRect.center().y() - renderSize.height() / 2), img.getTextureId());

	glDisable(GL_BLEND);
}
