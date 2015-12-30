#ifndef OPENGLRENDERVIEW_H
#define OPENGLRENDERVIEW_H

#include <QWidget>
#include <QGLWidget>

#include <panda/messaging.h>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
}

class OpenGLRenderView : public QGLWidget
{
	Q_OBJECT
public:
	explicit OpenGLRenderView(panda::PandaDocument* doc, QWidget* parent = nullptr);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void setAdjustRenderSize(bool adjust);

public slots:
	void renderSizeChanged();

protected:
	void initializeGL();
	void paintGL();

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

	void resizeEvent(QResizeEvent* event);

	panda::PandaDocument* m_document;
	bool m_adjustRenderSize;

	panda::msg::Observer m_observer;
};

#endif // OPENGLRENDERVIEW_H
