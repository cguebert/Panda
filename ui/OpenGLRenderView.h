#ifndef OPENGLRENDERVIEW_H
#define OPENGLRENDERVIEW_H

#include <QWidget>
#include <QGLWidget>

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
	explicit OpenGLRenderView(panda::PandaDocument* doc, QWidget *parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

signals:

public slots:

private:
	panda::PandaDocument* pandaDocument;
};

#endif // OPENGLRENDERVIEW_H
