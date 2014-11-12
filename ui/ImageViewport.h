#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include <panda/DataNode.h>

#include <QGLWidget>

namespace panda
{
class DataNode;
class BaseData;
}

class ImageViewport : public QGLWidget, public panda::DataNode
{
	Q_OBJECT
public:
	explicit ImageViewport(const panda::BaseData* data, QGLWidget* shareWidget, QWidget* parent = nullptr);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	virtual void update() {}
	virtual void setDirtyValue(const panda::DataNode* caller);
	virtual void doRemoveInput(panda::DataNode* node);

signals:
	void closeViewport(ImageViewport* viewport);

protected:
	void initializeGL();
	void paintGL();

	const panda::BaseData* m_data;
};

#endif // IMAGEVIEWPORT_H
