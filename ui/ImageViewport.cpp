#include <QtWidgets>
#include <ui/ImageViewport.h>

#include <panda/PandaDocument.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/graphics/Mat4x4.h>
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

ImageViewport::~ImageViewport()
{
	makeCurrent();
	m_texturedShader.clear();
	m_rectModel.clear();
	doneCurrent();
}

QSize ImageViewport::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize ImageViewport::sizeHint() const
{
	const ImageData* imageData = dynamic_cast<const ImageData*>(m_data);
	const ImageWrapper& img = imageData->getValue();
	if(img.isNull())
		return QSize(600, 400);
	return QSize(img.width(), img.height());
//	return QSize(600, 400);
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

void ImageViewport::initializeGL()
{
	m_texturedShader.addShaderFromMemory(panda::graphics::ShaderType::Vertex,
		panda::helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.v.glsl"));
	m_texturedShader.addShaderFromMemory(panda::graphics::ShaderType::Fragment,
		panda::helper::system::DataRepository.loadFile("shaders/PT_noColor_Tex.f.glsl"));
	m_texturedShader.link();
	m_texturedShader.bind();
	m_texturedShader.setUniformValue("tex0", 0);
	m_texturedShader.release();

	std::vector<GLfloat> verts = { 0, 0, 1, 0, 0, 1, 1, 1 };
	std::vector<GLfloat> texCoords = { 0, 1, 1, 1, 0, 0, 1, 0};
	m_rectModel.setVertices(verts);
	m_rectModel.setTexCoords(texCoords);
	m_rectModel.create();
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

	glViewport(0, 0, renderSize.width(), renderSize.height());
	panda::graphics::Mat4x4 mvp;
	GLfloat fw = static_cast<float>(renderSize.width()), fh = static_cast<float>(renderSize.height());
	mvp.ortho(0, fw, fh, 0, -10, 10);

	std::vector<GLfloat> verts = { 0, 0, fw, 0, 0, fh, fw, fh };
	m_rectModel.setVertices(verts);

	m_texturedShader.bind();
	m_texturedShader.setUniformValueMat4("MVP", mvp.data());

	glBindTexture(GL_TEXTURE_2D, img.getTextureId());

	m_rectModel.render();
	m_texturedShader.release();
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
