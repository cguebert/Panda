#ifndef OPENGLRENDERVIEW_H
#define OPENGLRENDERVIEW_H

#include <QOpenGLWidget>

#include <panda/messaging.h>
#include <panda/graphics/PointInt.h>

namespace panda
{
class BaseData;
class InteractiveDocument;
class PandaObject;
class RenderedDocument;
}

class OpenGLRenderView : public QOpenGLWidget
{
	Q_OBJECT
public:
	OpenGLRenderView(panda::RenderedDocument* doc, QWidget* parent = nullptr);

	QSize minimumSizeHint() const;

	void setAdjustRenderSize(bool adjust);

signals:
	void lostFocus(QWidget*);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void resizeEvent(QResizeEvent* event) override;
	void focusOutEvent(QFocusEvent*) override;

	panda::RenderedDocument* m_renderedDocument;
	panda::InteractiveDocument* m_interactiveDocument;
	bool m_adjustRenderSize;

	panda::msg::Observer m_observer;

	panda::graphics::PointInt m_deltaPos;
};

#endif // OPENGLRENDERVIEW_H
