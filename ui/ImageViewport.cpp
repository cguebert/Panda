#include <QtWidgets>
#include <ui/ImageViewport.h>
#include <ui/custom/DetachableTabWidget.h>

#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/helper/ShaderCache.h>
#include <panda/graphics/Mat4x4.h>
#include <panda/object/PandaObject.h>
#include <panda/types/ImageWrapper.h>
#include <panda/document/DocumentSignals.h>

#include <iostream>

using panda::Data;
using panda::types::ImageWrapper;
using ImageData = Data<ImageWrapper>;
using ImageListData = Data<std::vector<ImageWrapper>>;

ImageViewport::ImageViewport(const panda::BaseData* data, int imageIndex, QWidget* parent)
	: QOpenGLWidget(parent)
	, m_data(data)
	, m_owner(data->getOwner())
	, m_document(m_owner->parentDocument())
	, m_imageIndex(imageIndex)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	addInput(*const_cast<panda::BaseData*>(data));

	m_detachableWidgetInfo = new DetachableWidgetInfo(this);

	m_observer.get(m_document->getSignals().modifiedObject).connect<ImageViewport, &ImageViewport::onModifiedObject>(this);

	auto zoomInAction = new QAction(this);
	zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(zoomInAction, &QAction::triggered, this, &ImageViewport::zoomIn);
	addAction(zoomInAction);

	auto zoomOutAction = new QAction(this);
	zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(zoomOutAction, &QAction::triggered, this, &ImageViewport::zoomOut);
	addAction(zoomOutAction);

	auto resetZoomAction = new QAction(this);
	resetZoomAction->setShortcut(tr("Ctrl+0"));
	connect(resetZoomAction, &QAction::triggered, this, &ImageViewport::resetZoom);
	addAction(resetZoomAction);
}

ImageViewport::~ImageViewport()
{
	makeCurrent();
	m_texturedShader.clear();
	m_rectModel.clear();
	doneCurrent();

	emit destroyedViewport(this);
}

QSize ImageViewport::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize ImageViewport::sizeHint() const
{
	return QSize(600, 400); // If we ask the size of the image here, we could create the OpenGL objects in a wrong context
}

void ImageViewport::setDirtyValue(const panda::DataNode* /*caller*/)
{
	auto document = m_data->getOwner()->parentDocument();
	if (!m_data->isDirty() // Just got modified
		|| !document->animationIsPlaying()) // Or animation not playing
		updateData();
}

void ImageViewport::updateData()
{
	auto document = m_data->getOwner()->parentDocument();
	document->getGUI().executeByUI([this, document]() { // Execute this outside of any rendering
		if (!m_data)
			return;

		if (m_data->isDirty())
		{
			// Update the data
			auto& gui = document->getGUI();
			gui.contextMakeCurrent();
			m_data->updateIfDirty();
			gui.contextDoneCurrent();
		}

		// Resize the widget
		QSize s = size();
		auto img = getImage();
		if(!img || img->isNull())
			resize(600, 400);
		else
			resize(img->width(), img->height());

		// Ask for a redraw
		QWidget::update();
	});
}

void ImageViewport::doRemoveInput(DataNode& node)
{
	DataNode::doRemoveInput(node);
	if(&node == m_data)
	{
		m_data = nullptr;
		emit closeViewport(this);
	}
}

void ImageViewport::initializeGL()
{
	using ShaderType = panda::graphics::ShaderType;
	m_texturedShader = panda::helper::ShaderCache::getInstance()->getShaderProgramFromFile( { 
		{ ShaderType::Vertex, "shaders/PT_noColor_Tex.v.glsl" }, 
		{ ShaderType::Fragment, "shaders/PT_noColor_Tex.f.glsl" } 
	} );
	m_texturedShader.bind();
	m_texturedShader.setUniformValue("tex0", 0);
	m_texturedShader.release();

	std::vector<GLfloat> verts = { 0, 0, 1, 0, 0, 1, 1, 1 };
	std::vector<GLfloat> texCoords = { 0, 1, 1, 1, 0, 0, 1, 0};
	m_rectModel.setVertices(verts);
	m_rectModel.setTexCoords(texCoords);
	m_rectModel.create();

	updateData();
}

void ImageViewport::paintGL()
{
	QColor col = palette().window().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glDepthMask( GL_TRUE );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto img = getImage();
	if(!img || img->isNull())
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto imgSize = img->size();
	auto renderSize = imgSize * m_zoomFactor;

	if (size() != QSize(renderSize.width(), renderSize.height()))
		resize(renderSize.width(), renderSize.height());

	glViewport(0, 0, renderSize.width(), renderSize.height());
	panda::graphics::Mat4x4 mvp;
	GLfloat fw = static_cast<float>(renderSize.width()), fh = static_cast<float>(renderSize.height());
	mvp.ortho(0, fw, fh, 0, -10, 10);

	std::vector<GLfloat> verts = { 0, 0, fw, 0, 0, fh, fw, fh };
	m_rectModel.setVertices(verts);

	m_texturedShader.bind();
	m_texturedShader.setUniformValueMat4("MVP", mvp.data());

	glBindTexture(GL_TEXTURE_2D, img->getTextureId());

	m_rectModel.render();
	m_texturedShader.release();

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
		setImageTitle();
	}
}

const panda::types::ImageWrapper* ImageViewport::getImage() const
{
	if (!m_data)
		return nullptr;

	const ImageData* imageData = dynamic_cast<const ImageData*>(m_data);
	if(imageData)
		return &imageData->getValue();

	const ImageListData* imagesListData = dynamic_cast<const ImageListData*>(m_data);
	if (imagesListData)
	{
		auto& imagesList = imagesListData->getValue();
		int nb = imagesList.size();
		if (m_imageIndex < 0 || m_imageIndex >= nb)
			return nullptr;
		return &imagesList[m_imageIndex];
	}

	return nullptr;
}

void ImageViewport::setImageTitle()
{
	QString label = QString::fromStdString(m_data->getOwner()->getName()) + "." + QString::fromStdString(m_data->getName());

	const ImageListData* imagesListData = dynamic_cast<const ImageListData*>(m_data);
	if (imagesListData)
	{
		auto& imagesList = imagesListData->getValue();
		int nb = imagesList.size();
		if (m_imageIndex >= 0 || m_imageIndex < nb)
			label += QString(" [%1/%2]").arg(m_imageIndex + 1).arg(nb);
	}

	if (m_zoomFactor < 1)
		label += QString(" %1%").arg(lround(m_zoomFactor * 100));

	m_detachableWidgetInfo->changeTitle(label);
}

void ImageViewport::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers() != Qt::ControlModifier)
	{
		QWidget::keyPressEvent(event);
		return;
	}

	switch (event->key())
	{
	case Qt::Key_PageDown:
	{
		const ImageListData* imagesListData = dynamic_cast<const ImageListData*>(m_data);
		if (imagesListData)
		{
			auto& imagesList = imagesListData->getValue();
			int nb = imagesList.size();

			++m_imageIndex;
			if (m_imageIndex >= nb)
				m_imageIndex = nb - 1;
			updateData();
			setImageTitle();
		}
		break;
	}
	case Qt::Key_PageUp:
	{
		const ImageListData* imagesListData = dynamic_cast<const ImageListData*>(m_data);
		if (imagesListData)
		{
			auto& imagesList = imagesListData->getValue();
			int nb = imagesList.size();

			--m_imageIndex;
			if (m_imageIndex < 0)
				m_imageIndex = 0;
			updateData();
			setImageTitle();
		}
		break;
	}
	default:
		QWidget::keyPressEvent(event);
	}
}

void ImageViewport::onModifiedObject(panda::PandaObject* object)
{
	if (!m_data || object != m_owner)
		return;

	if (m_data->isInput() && !m_data->getParent())
	{
		m_data = nullptr;
		emit closeViewport(this);
	}
}

void ImageViewport::zoomIn()
{
	int newZoom = qBound(0, m_zoomLevel - 5, 90);
	if (m_zoomLevel != newZoom)
	{
		m_zoomLevel = newZoom;
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		QWidget::update();
		setImageTitle();
	}
}

void ImageViewport::zoomOut()
{
	int newZoom = qBound(0, m_zoomLevel + 5, 90);
	if (m_zoomLevel != newZoom)
	{
		m_zoomLevel = newZoom;
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		QWidget::update();
		setImageTitle();
	}
}

void ImageViewport::resetZoom()
{
	if (m_zoomLevel != 0)
	{
		m_zoomLevel = 0;
		m_zoomFactor = 1.f;
		QWidget::update();
		setImageTitle();
	}
}

void ImageViewport::focusOutEvent(QFocusEvent*)
{
	emit lostFocus(this);
}
