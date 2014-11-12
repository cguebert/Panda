#ifndef DETACHABLETABWIDGET_H
#define DETACHABLETABWIDGET_H

#include <QDialog>
#include <QTabBar>
#include <QTabWidget>
#include <QMap>

class QVBoxLayout;
class DetachedWindow;

class DetachableTabBar : public QTabBar
{
	Q_OBJECT
public:
	explicit DetachableTabBar(QWidget* parent = nullptr);

signals:
	void moveTab(int from, int to);
	void detachTab(int id);

protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);

	QPoint m_dragStart, m_dragDrop;
	bool m_dragging;
};

//****************************************************************************//

// TODO: add a ptr to the "main" tabwidget
class DetachableTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit DetachableTabWidget(QWidget* parent = nullptr);

	int addTab(QWidget* widget, const QString& label, bool closable = false); // Replacing QTabWidget::addTab functions

	struct TabInfo
	{
		TabInfo() : widget(nullptr), closable(false) {}
		TabInfo(QWidget* w, const QString& t, bool c)
			: widget(w), title(t), closable(c) {}
		QWidget* widget;
		QString title;
		bool closable;
	};

signals:
	void openDetachedWindow(DetachedWindow* window);

public slots:
	void moveTab(int from, int to);
	void detachTab(int id);
	void attachTab(DetachableTabWidget::TabInfo tabInfo);
	void closeTab(int id);

protected:
	DetachableTabBar* m_tabBar;
	QMap<QWidget*, TabInfo> m_tabsInfo;
};

//****************************************************************************//

// TODO: add a tabwidget so that we can have multiple tabs in a window
class DetachedWindow : public QDialog
{
	Q_OBJECT
public:
	DetachedWindow(QWidget* parent = nullptr);

	void attachTab(DetachableTabWidget::TabInfo tabInfo);
	void closeTab();
	DetachableTabWidget::TabInfo getTabInfo() const;

signals:
	void detachTab(DetachableTabWidget::TabInfo tabInfo);
	void closeDetachedWindow(DetachedWindow* window);

protected:
	void closeEvent(QCloseEvent* event);

	QVBoxLayout* m_mainLayout;
	DetachableTabWidget::TabInfo m_tabContent;
};

#endif // DETACHABLETABWIDGET_H
