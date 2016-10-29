#pragma once

#include <QOpenGLWidget>

#include <functional>
#include <map>
#include <memory>
#include <set>

#include <ui/custom/ScrollContainer.h>
#include <ui/graphview/graphics/DrawList.h>
#include <ui/graphview/graphics/DrawColors.h>
#include <panda/messaging.h>

class MainWindow;

namespace panda
{
	class PandaObject;
}

namespace graphview
{

	class GraphView;

	class QtViewWrapper : public QOpenGLWidget, public ScrollableView
	{
		Q_OBJECT

	public:
		explicit QtViewWrapper(std::unique_ptr<GraphView> graphView, MainWindow* mainWindow);

		GraphView& view() const;

		void executeNextRefresh(std::function<void()> func);

	public slots:
		void copy();
		void cut();
		void paste();
		void del();
		void showChooseWidgetDialog();
		void debugDirtyState(bool show = true);
		void setDataLabel();

	signals:
		void modified();
		void viewportModified();
		void lostFocus(QWidget*);

	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;

		QSize minimumSizeHint() const override;
		QSize sizeHint() const override;

		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void contextMenuEvent(QContextMenuEvent* event) override;
		void focusOutEvent(QFocusEvent*) override;

		// From ScrollableView
		QSize viewSize() override;
		QPoint viewPosition() override;
		void scrollView(QPoint position) override;

#ifdef PANDA_LOG_EVENTS
		void paintLogDebug(graphics::DrawList& list, graphics::DrawColors& colors);
#endif
		void paintDirtyState(graphics::DrawList& list, graphics::DrawColors& colors);

	protected:

		std::unique_ptr<GraphView> m_graphView;
		std::shared_ptr<ViewRenderer> m_viewRenderer;
		std::vector<std::function<void()>> m_functionsToExecuteNextRefresh;
		bool m_debugDirtyState = false;

		std::shared_ptr<graphics::DrawList> m_drawList;
		graphics::DrawColors m_drawColors; /// So that we aquire Qt colors only once
	};

	//****************************************************************************//

	inline GraphView& QtViewWrapper::view() const
	{ return *m_graphView; }

	inline void QtViewWrapper::debugDirtyState(bool show)
	{ m_debugDirtyState = show; update(); }

} // namespace graphview
