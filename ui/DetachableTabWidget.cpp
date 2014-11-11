#include <ui/DetachableTabWidget.h>

#include <QtWidgets>

#include <iostream>

DetachableTabBar::DetachableTabBar(QWidget* parent)
	: QTabBar(parent)
{
	setAcceptDrops(true);
	setMovable(true);
}

void DetachableTabBar::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
		m_dragStart = event->pos();

	m_dragDrop = QPoint();
	m_dragging = false;

	QTabBar::mousePressEvent(event);
}

void DetachableTabBar::mouseMoveEvent(QMouseEvent* event)
{
	if(!m_dragging && !m_dragStart.isNull() && (event->buttons() & Qt::LeftButton) && count() > 1
			&& (event->pos() - m_dragStart).manhattanLength() > QApplication::startDragDistance())
		m_dragging = true;

	if(m_dragging && !geometry().contains(event->pos()))
	{
		// Stop the move
		QMouseEvent finishMoveEvent(QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
		QTabBar::mouseMoveEvent(&finishMoveEvent);

		// Prepare the drag action
		QDrag dragAction(this);
		QMimeData* mime = new QMimeData();
		mime->setData("action", "application/tab-detach");
		dragAction.setMimeData(mime);

		// Create a image of the contents of the tab
		QPixmap pixmap = dynamic_cast<DetachableTabWidget*>(parentWidget())->currentWidget()->grab()
				.scaled(640, 480, Qt::KeepAspectRatio);
		QPixmap targetPixmap(pixmap.size ());
		QPainter painter(&targetPixmap);
		painter.setOpacity(0.4);
		painter.drawPixmap(0,0, pixmap);
		painter.end();
		dragAction.setPixmap(targetPixmap);

		// Start the drag action
		Qt::DropAction dragged = dragAction.exec(Qt::MoveAction);
		if(dragged == Qt::IgnoreAction)
		{
			event->accept();
			detachTab(tabAt(m_dragStart));
		}
		else if(dragged == Qt::MoveAction)
		{
			if(!m_dragDrop.isNull())
			{
				event->accept();
				moveTab(tabAt(m_dragStart), tabAt(m_dragDrop));
			}
		}
	}
	else
		QTabBar::mouseMoveEvent(event);
}

void DetachableTabBar::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if(mime->formats().contains("action") && mime->data("action") == "application/tab-detach")
		event->acceptProposedAction();

	QTabBar::dragEnterEvent(event);
}

void DetachableTabBar::dragMoveEvent(QDragMoveEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if(mime->formats().contains("action") && mime->data("action") == "application/tab-detach")
		event->acceptProposedAction();

	QTabBar::dragMoveEvent(event);
}

void DetachableTabBar::dropEvent(QDropEvent* event)
{
	if(geometry().contains(event->pos()))
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
	// TODO: is it the same tabbar or in another window?
	m_dragDrop = event->pos();
	QTabBar::dropEvent(event);
}

//****************************************************************************//

DetachableTabWidget::DetachableTabWidget(QWidget* parent)
	: QTabWidget(parent)
{
	m_tabBar = new DetachableTabBar(this);
	setTabBar(m_tabBar);

	connect(m_tabBar, SIGNAL(moveTab(int,int)), this, SLOT(moveTab(int,int)));
	connect(m_tabBar, SIGNAL(detachTab(int)), this, SLOT(detachTab(int)));
}

void DetachableTabWidget::moveTab(int from, int to)
{
	QWidget* tab = widget(from);
	QIcon icon = tabIcon(from);
	QString text = tabText(from);

	removeTab(from);
	insertTab(to, tab, icon, text);
	setCurrentIndex(to);
}

void DetachableTabWidget::detachTab(int id)
{
	DetachedWindow* detachedWindow = new DetachedWindow(parentWidget());
	detachedWindow->setWindowTitle(tabText(id));
	connect(detachedWindow, SIGNAL(detachTab(QWidget*,QString)), this, SLOT(attachTab(QWidget*,QString)));

	QWidget* tabContent = widget(id);
	detachedWindow->attachTab(tabContent); // The setParent inside will remove the tab
	tabContent->show(); // Have to call it manually
	detachedWindow->resize(tabContent->sizeHint().expandedTo(QSize(640, 480)));
	detachedWindow->show();

	update();
}

void DetachableTabWidget::attachTab(QWidget* widget, QString title)
{
	widget->setParent(this);
	addTab(widget, title);
}

//****************************************************************************//

DetachedWindow::DetachedWindow(QWidget* parent)
	: QDialog(parent)
	, m_tabContent(nullptr)
{
	m_mainLayout = new QVBoxLayout(this);
	setLayout(m_mainLayout);
	setAttribute(Qt::WA_DeleteOnClose);
}

void DetachedWindow::attachTab(QWidget* widget)
{
	widget->setParent(this);
	m_mainLayout->addWidget(widget);
	m_tabContent = widget;
}

void DetachedWindow::closeEvent(QCloseEvent* /*event*/)
{
	// For all tabs in tabbar, emit attachTab
	emit detachTab(m_tabContent, windowTitle());
}
