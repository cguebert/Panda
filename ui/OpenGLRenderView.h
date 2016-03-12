#ifndef OPENGLRENDERVIEW_H
#define OPENGLRENDERVIEW_H

#include <QOpenGLWidget>

#include <panda/messaging.h>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
}

class OpenGLRenderView : public QOpenGLWidget
{
	Q_OBJECT
public:
	OpenGLRenderView(panda::PandaDocument* doc, QWidget* parent = nullptr);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void setAdjustRenderSize(bool adjust);

public slots:
	void renderSizeChanged();

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

	panda::PandaDocument* m_document;
	bool m_adjustRenderSize;

	panda::msg::Observer m_observer;
};

#endif // OPENGLRENDERVIEW_H
