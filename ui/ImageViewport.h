#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include <panda/data/DataNode.h>
#include <panda/graphics/Model.h>
#include <panda/graphics/ShaderProgram.h>

#include <QOpenGLWidget>

namespace panda
{
class DataNode;
class BaseData;

namespace types
{ class ImageWrapper; }

}

class ImageViewport : public QOpenGLWidget, public panda::DataNode
{
	Q_OBJECT
public:
	explicit ImageViewport(const panda::BaseData* data, int imageIndex = 0, QWidget* parent = nullptr);
	~ImageViewport();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void update() override {}
	void setDirtyValue(const panda::DataNode* caller) override;
	void doRemoveInput(panda::DataNode& node) override;

signals:
	void closeViewport(ImageViewport* viewport); // Ask the main window to close this viewport (and potentially its parent detached window)
	void destroyedViewport(ImageViewport* viewport); // Tell the main window this viewport is freed (called by the destructor)

protected:
	void initializeGL() override;
	void paintGL() override;

	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

	void updateData();
	const panda::types::ImageWrapper* getImage() const;

	const panda::BaseData* m_data;
	int m_zoomLevel = 0, m_wheelTicks = 0, m_imageIndex = -1;
	float m_zoomFactor = 1.f;

	panda::graphics::ShaderProgram m_texturedShader;
	panda::graphics::Model m_rectModel;
};

#endif // IMAGEVIEWPORT_H
