#pragma once

#include <QOpenGLWidget>

#include <functional>
#include <map>
#include <memory>
#include <set>

#include <ui/custom/ScrollContainer.h>
#include <panda/graphview/graphics/DrawList.h>
#include <panda/graphview/graphics/DrawColors.h>
#include <panda/messaging.h>

class MainWindow;

namespace panda
{
	class PandaObject;

	namespace graphview {
		class GraphView;
	}
}

namespace graphview
{

	class QtViewWrapper : public QOpenGLWidget, public ScrollableView
	{
		Q_OBJECT

	public:
		explicit QtViewWrapper(std::unique_ptr<panda::graphview::GraphView> graphView, MainWindow* mainWindow);

		panda::graphview::GraphView& view() const;

		void executeNextRefresh(std::function<void()> func);

	public slots:
		void copy();
		void cut();
		void paste();
		void del();
		void showChooseWidgetDialog();
		void debugDirtyState(bool show = true);
		void setDataLabel();
		void createVisualizer();
		void saveGroup();

	signals:
		void viewportModified();
		void modified();
		void lostFocus(QWidget*);
		void groupsListModified();

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
		void paintLogDebug(panda::graphview::graphics::DrawList& list, panda::graphview::graphics::DrawColors& colors);
#endif
		void paintDirtyState(panda::graphview::graphics::DrawList& list, panda::graphview::graphics::DrawColors& colors);

		void emitViewportModified();

	protected:
		std::unique_ptr<panda::graphview::GraphView> m_graphView;
		std::shared_ptr<panda::graphview::ViewRenderer> m_viewRenderer;
		std::vector<std::function<void()>> m_functionsToExecuteNextRefresh;
		bool m_debugDirtyState = false;

		panda::msg::Observer m_observer;

		std::shared_ptr<panda::graphview::graphics::DrawList> m_drawList;
		panda::graphview::graphics::DrawColors m_drawColors; /// So that we aquire Qt colors only once
	};

	//****************************************************************************//

	inline panda::graphview::GraphView& QtViewWrapper::view() const
	{ return *m_graphView; }

	inline void QtViewWrapper::debugDirtyState(bool show)
	{ m_debugDirtyState = show; update(); }

} // namespace graphview
