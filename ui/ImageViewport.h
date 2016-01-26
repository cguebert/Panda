#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include <panda/data/DataNode.h>

#include <QOpenGLWidget>

namespace panda
{
class DataNode;
class BaseData;
}

class ImageViewport : public QOpenGLWidget, public panda::DataNode
{
	Q_OBJECT
public:
	explicit ImageViewport(const panda::BaseData* data, QWidget* parent = nullptr);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void update() override {}
	void setDirtyValue(const panda::DataNode* caller) override;
	void doRemoveInput(panda::DataNode& node) override;

signals:
	void closeViewport(ImageViewport* viewport);

protected:
	void paintGL() override;

	virtual void wheelEvent(QWheelEvent* event);

	const panda::BaseData* m_data;
	int m_zoomLevel, m_wheelTicks;
	float m_zoomFactor;
};

#endif // IMAGEVIEWPORT_H
