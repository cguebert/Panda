#ifndef OPENGLRENDERVIEW_H
#define OPENGLRENDERVIEW_H

#include <QWindow>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
}

class QOpenGLContext;
class QOpenGLShaderProgram;

class OpenGLRenderView : public QWindow
{
	Q_OBJECT
public:
	static void createOpenGLRenderView(panda::PandaDocument* document,
									   OpenGLRenderView*& view,
									   QWidget*& container);

	explicit OpenGLRenderView(panda::PandaDocument* document, QWindow* parent = nullptr);
	~OpenGLRenderView();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void setAdjustRenderSize(bool adjust);
	void setContainer(QWidget* container);

	void update();

protected:
	void render();

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	bool event(QEvent* event);
	void resizeEvent(QResizeEvent* event);

private:
	panda::PandaDocument* m_document;
	QWidget* m_containingWidget;
	QOpenGLContext* m_context;
	QOpenGLShaderProgram* m_shaderProgram;

	bool m_adjustRenderSize;
	bool m_updatePending;
};

#endif // OPENGLRENDERVIEW_H
