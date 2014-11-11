#ifndef DETACHABLETABWIDGET_H
#define DETACHABLETABWIDGET_H

#include <QDialog>
#include <QTabBar>
#include <QTabWidget>

class QVBoxLayout;

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

public slots:
	void moveTab(int from, int to);
	void detachTab(int id);
	void attachTab(QWidget* widget, QString title);

protected:
	DetachableTabBar* m_tabBar;
};

//****************************************************************************//

// TODO: add a tabwidget so that we can have multiple tabs in a window
class DetachedWindow : public QDialog
{
	Q_OBJECT
public:
	DetachedWindow(QWidget* parent = nullptr);

	void attachTab(QWidget* widget);

signals:
	void detachTab(QWidget* widget, QString title);

protected:
	void closeEvent(QCloseEvent* event);

	QVBoxLayout* m_mainLayout;
	QWidget* m_tabContent;
};

#endif // DETACHABLETABWIDGET_H
