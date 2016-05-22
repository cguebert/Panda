#ifndef DETACHABLETABWIDGET_H
#define DETACHABLETABWIDGET_H

#include <QDialog>
#include <QTabBar>
#include <QTabWidget>
#include <map>

class QVBoxLayout;
class DetachedWindow;

class DetachableWidgetInfo : public QObject
{
	Q_OBJECT
public:
	explicit DetachableWidgetInfo(QWidget* parent = nullptr)
		: QObject(parent) {}

	void changeTitle(QString title);

signals:
	void changedTitle(DetachableWidgetInfo* thisObject, QString title);
};

//****************************************************************************//

class DetachableTabBar : public QTabBar
{
	Q_OBJECT
public:
	explicit DetachableTabBar(QWidget* parent = nullptr);

signals:
	void moveTab(int from, int to);
	void detachTab(int id);
	void middleClicked(int id);

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

	int addTab(QWidget* widget, const QString& label, DetachableWidgetInfo* info = nullptr); // Replacing QTabWidget::addTab functions

	struct TabInfo
	{
		TabInfo() = default;
		TabInfo(QWidget* w, const QString& t, DetachableWidgetInfo* i)
			: widget(w), title(t), info(i) {}
		QWidget* widget = nullptr;
		DetachableWidgetInfo* info = nullptr;
		QString title;
		bool closable = false;
	};

signals:
	void openDetachedWindow(DetachedWindow* window);
	void closedTab(QWidget* widget);

public slots:
	void moveTab(int from, int to);
	void detachTab(int id);
	void attachTab(DetachableTabWidget::TabInfo tabInfo);
	void closeTab(int id);
	void renameTab(DetachableWidgetInfo* info, QString title);
	void middleClicked(int id);

protected:
	DetachableTabBar* m_tabBar;
	std::map<QWidget*, TabInfo> m_tabsInfo;
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
	void changeTitle(DetachableWidgetInfo*, QString title);

	QVBoxLayout* m_mainLayout;
	DetachableTabWidget::TabInfo m_tabContent;
};

#endif // DETACHABLETABWIDGET_H
