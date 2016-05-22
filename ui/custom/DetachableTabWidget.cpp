#include <ui/custom/DetachableTabWidget.h>

#include <QtWidgets>

#include <iostream>

void DetachableWidgetInfo::changeTitle(QString title)
{
	emit changedTitle(this, title);
}

//****************************************************************************//

DetachableTabBar::DetachableTabBar(QWidget* parent)
	: QTabBar(parent)
{
	setAcceptDrops(true);
	setMovable(true);
	setTabsClosable(true);
}

void DetachableTabBar::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
		m_dragStart = event->pos();
	else if (event->button() == Qt::MiddleButton)
	{
		int id = tabAt(event->pos());
		if (id != -1)
			emit middleClicked(id);
		return;
	}

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
	connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
	connect(m_tabBar, SIGNAL(middleClicked(int)), this, SLOT(middleClicked(int)));
}

int DetachableTabWidget::addTab(QWidget* widget, const QString& label, DetachableWidgetInfo* info)
{
	int id = QTabWidget::addTab(widget, label);

	if(info)
		connect(info, &DetachableWidgetInfo::changedTitle, this, &DetachableTabWidget::renameTab);
	else
	{	// Remove close button
		m_tabBar->setTabButton(id, QTabBar::RightSide, nullptr);
		m_tabBar->setTabButton(id, QTabBar::LeftSide, nullptr);
	}		

	m_tabsInfo[widget] = TabInfo(widget, label, info);
	
	return id;
}

void DetachableTabWidget::moveTab(int from, int to)
{
	QWidget* tab = widget(from);
	QIcon icon = tabIcon(from);
	QString text = tabText(from);

	removeTab(from);
	insertTab(to, tab, icon, text);
	setCurrentIndex(to);

	if (!m_tabsInfo[tab].info)
	{
		m_tabBar->setTabButton(to, QTabBar::RightSide, nullptr);
		m_tabBar->setTabButton(to, QTabBar::LeftSide, nullptr);
	}
}

void DetachableTabWidget::detachTab(int id)
{
	DetachedWindow* detachedWindow = new DetachedWindow(parentWidget());
	connect(detachedWindow, SIGNAL(detachTab(DetachableTabWidget::TabInfo)), this, SLOT(attachTab(DetachableTabWidget::TabInfo)));

	QWidget* w = widget(id);
	w->disconnect(this);
	detachedWindow->attachTab(m_tabsInfo[w]); // The setParent inside will remove the tab
	w->show(); // Have to call it manually
	detachedWindow->resize(w->sizeHint().expandedTo(QSize(640, 480)));
	detachedWindow->show();

	emit openDetachedWindow(detachedWindow);

	update();
}

void DetachableTabWidget::attachTab(TabInfo tabInfo)
{
	tabInfo.widget->setParent(this);
	addTab(tabInfo.widget, tabInfo.title, tabInfo.info);
}

void DetachableTabWidget::closeTab(int id)
{
	QWidget* w = widget(id);
	removeTab(id);
	w->deleteLater();
	m_tabsInfo.erase(w);

	emit closedTab(w);
}

void DetachableTabWidget::renameTab(DetachableWidgetInfo* info, QString title)
{
	auto it = std::find_if(m_tabsInfo.begin(), m_tabsInfo.end(), [info](const auto& ti) {
		return ti.second.info == info;
	});
	if (it == m_tabsInfo.end())
		return;

	it->second.title = title;
	int index = indexOf(it->second.widget);
	if(index != -1)
		setTabText(index, title);
}

void DetachableTabWidget::middleClicked(int id)
{
	if (m_tabsInfo[widget(id)].info)
		closeTab(id);
}

//****************************************************************************//

DetachedWindow::DetachedWindow(QWidget* parent)
	: QDialog(parent)
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	setLayout(m_mainLayout);
	setAttribute(Qt::WA_DeleteOnClose);
}

void DetachedWindow::attachTab(DetachableTabWidget::TabInfo tabInfo)
{
	setWindowTitle(tabInfo.title);
	tabInfo.widget->setParent(this);
	m_mainLayout->addWidget(tabInfo.widget);
	m_tabContent = tabInfo;

	if(tabInfo.info)
		connect(tabInfo.info, &DetachableWidgetInfo::changedTitle, this, &DetachedWindow::changeTitle);
}

void DetachedWindow::closeTab()
{
	m_tabContent.widget = nullptr; // Still a child of this window, and will be deleted by it
	close();
}

DetachableTabWidget::TabInfo DetachedWindow::getTabInfo() const
{
	return m_tabContent;
}

void DetachedWindow::closeEvent(QCloseEvent* /*event*/)
{
	// For all tabs in tabbar, emit attachTab
	if(m_tabContent.widget)
		emit detachTab(m_tabContent);
	emit closeDetachedWindow(this);
}

void DetachedWindow::changeTitle(DetachableWidgetInfo*, QString title)
{
	setWindowTitle(title);
	m_tabContent.title = title;
}
