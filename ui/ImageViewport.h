#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include <panda/data/DataNode.h>
#include <panda/graphics/Model.h>
#include <panda/graphics/ShaderProgram.h>
#include <panda/messaging.h>

#include <QOpenGLWidget>

namespace panda
{

class BaseData;
class DataNode;
class PandaDocument;
class PandaObject;

namespace types
{ class ImageWrapper; }

}

class DetachableWidgetInfo;

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

	DetachableWidgetInfo* getDetachableWidgetInfo();

signals:
	void closeViewport(ImageViewport* viewport); // Ask the main window to close this viewport (and potentially its parent detached window)
	void lostFocus(QWidget*);

protected:
	void initializeGL() override;
	void paintGL() override;

	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void focusOutEvent(QFocusEvent*) override;

	void zoomIn();
	void zoomOut();
	void resetZoom();

	void updateData();
	const panda::types::ImageWrapper* getImage() const;
	void setImageTitle();

	void onModifiedObject(panda::PandaObject* object);

	const panda::BaseData* m_data = nullptr;
	panda::PandaObject* m_owner = nullptr;
	panda::PandaDocument* m_document = nullptr;

	int m_zoomLevel = 0, m_wheelTicks = 0, m_imageIndex = -1;
	float m_zoomFactor = 1.f;

	panda::graphics::ShaderProgram m_texturedShader;
	panda::graphics::Model m_rectModel;

	DetachableWidgetInfo* m_detachableWidgetInfo;

	panda::msg::Observer m_observer;
};

inline DetachableWidgetInfo* ImageViewport::getDetachableWidgetInfo()
{ return m_detachableWidgetInfo; }

#endif // IMAGEVIEWPORT_H
