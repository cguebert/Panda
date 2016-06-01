#ifndef DETACHABLETABWIDGET_H
#define DETACHABLETABWIDGET_H

#include <QDialog>
#include <QTabBar>
#include <QTabWidget>
#include <map>

class QVBoxLayout;
class DetachedWindow;
class MainWindow;

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

	void setDropDestination(QWidget* widget, QPoint position);

signals:
	void middleClicked(int id);
	void dropTab(int from, QWidget* dstWidget, QPoint pos);

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private:
	QPoint m_dragStart, m_dropPosition;
	QWidget* m_dropWidget = nullptr;
	bool m_dragging;
};

//****************************************************************************//

class DetachableTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit DetachableTabWidget(MainWindow* mainWindow, QWidget* parent = nullptr);

	int addTab(QWidget* widget, const QString& label, DetachableWidgetInfo* info = nullptr); // Replacing QTabWidget::addTab functions

	struct TabInfo
	{
		TabInfo() = default;
		TabInfo(QWidget* w, const QString& t, DetachableWidgetInfo* i)
			: widget(w), title(t), info(i) {}
		QWidget* widget = nullptr;
		DetachableWidgetInfo* info = nullptr;
		QString title;
	};

	TabInfo getInfo(int id);

signals:
	void closedTab(QWidget* widget);
	void removedTab(QWidget* widget);

public slots:
	void attachTab(DetachableTabWidget::TabInfo tabInfo);
	void closeTab(int id);
	void renameTab(DetachableWidgetInfo* info, QString title);
	void middleClicked(int id);
	void dropTab(int from, QWidget* dstWidget, QPoint pos);

private:
	void moveTab(int from, int to);
	void moveTab(int id, DetachableTabWidget* dstWidget);
	void moveTab(int id, DetachedWindow* dstWindow);
	void detachTab(int id);

	MainWindow* m_mainWindow;
	DetachableTabBar* m_tabBar;
	std::map<QWidget*, TabInfo> m_tabsInfo;
};

//****************************************************************************//

// TODO: add a tabwidget so that we can have multiple tabs in a window
class DetachedWindow : public QDialog
{
	Q_OBJECT
public:
	DetachedWindow(MainWindow* parent = nullptr);

	void attachTab(DetachableTabWidget::TabInfo tabInfo);
	bool closeTab(QWidget* tab);
	DetachableTabWidget::TabInfo getTabInfo() const;

signals:
	void detachTab(DetachableTabWidget::TabInfo tabInfo);
	void closedTab(QWidget* widget);
	void closedDetachedWindow(DetachedWindow* window);

private:
	void createTabWidget();
	void removeTabWidget();

	void changeTitle(DetachableWidgetInfo*, QString title);

	void closeEvent(QCloseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	MainWindow* m_mainWindow;
	QVBoxLayout* m_mainLayout;
	DetachableTabWidget* m_tabWidget = nullptr;
	DetachableTabWidget::TabInfo m_defaultTabContent;
};

#endif // DETACHABLETABWIDGET_H
