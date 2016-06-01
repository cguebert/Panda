#include <ui/custom/DetachableTabWidget.h>
#include <ui/MainWindow.h>

#include <QtWidgets>

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
	if (event->button() == Qt::LeftButton)
	{
		m_dragStart = event->pos();
		m_dropPosition = QPoint();
		m_dragging = false;
	}
	else if (event->button() == Qt::MiddleButton)
	{
		int id = tabAt(event->pos());
		if (id != -1)
			emit middleClicked(id);
		return;
	}

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

		event->accept(); // Break the mouse move event
		emit dropTab(tabAt(m_dragStart), m_dropWidget, m_dropPosition); // The drop action is now done by the tab widget

		m_dragging = false;
		m_dropWidget = nullptr;
	}
	else
		QTabBar::mouseMoveEvent(event);
}

void DetachableTabBar::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if(mime->formats().contains("action") && mime->data("action") == "application/tab-detach")
		event->acceptProposedAction();
}

void DetachableTabBar::dragMoveEvent(QDragMoveEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if(mime->formats().contains("action") && mime->data("action") == "application/tab-detach")
		event->acceptProposedAction();
}

void DetachableTabBar::dropEvent(QDropEvent* event)
{
	if(geometry().contains(event->pos()))
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();

		if (auto source = qobject_cast<DetachableTabBar*>(event->source()))
			source->setDropDestination(this, event->pos());
	}
}

void DetachableTabBar::setDropDestination(QWidget* widget, QPoint position)
{
	m_dropWidget = widget;
	m_dropPosition = position;
}

//****************************************************************************//

DetachableTabWidget::DetachableTabWidget(MainWindow* mainWindow, QWidget* parent)
	: QTabWidget(parent)
	, m_mainWindow(mainWindow)
{
	m_tabBar = new DetachableTabBar(this);
	setTabBar(m_tabBar);

	connect(m_tabBar, &DetachableTabBar::tabCloseRequested, this, &DetachableTabWidget::closeTab);
	connect(m_tabBar, &DetachableTabBar::middleClicked, this, &DetachableTabWidget::middleClicked);
	connect(m_tabBar, &DetachableTabBar::dropTab, this, &DetachableTabWidget::dropTab);
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

DetachableTabWidget::TabInfo DetachableTabWidget::getInfo(int id)
{
	auto w = widget(id);
	return m_tabsInfo.at(w);
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

void DetachableTabWidget::moveTab(int id, DetachableTabWidget* dstWidget)
{
	QWidget* w = widget(id);
	QString label = tabText(id);
	w->disconnect(this);
	dstWidget->addTab(w, label, m_tabsInfo[w].info);
	m_tabsInfo.erase(w);
	emit removedTab(w);
}

void DetachableTabWidget::moveTab(int id, DetachedWindow* dstWindow)
{
	QWidget* w = widget(id);
	QString label = tabText(id);
	w->disconnect(this);
	dstWindow->attachTab(m_tabsInfo[w]);
	m_tabsInfo.erase(w);
	w->show();
	emit removedTab(w);
}

void DetachableTabWidget::detachTab(int id)
{
	auto detachedWindow = m_mainWindow->openDetachedWindow();
	QWidget* w = widget(id);
	w->disconnect(this);
	detachedWindow->attachTab(m_tabsInfo[w]); // The setParent inside will remove the tab
	m_tabsInfo.erase(w);
	w->show(); // Have to call it manually
	detachedWindow->resize(w->sizeHint().expandedTo(QSize(640, 480)));
	detachedWindow->show();

	emit removedTab(w);
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

void DetachableTabWidget::dropTab(int from, QWidget* dstWidget, QPoint pos)
{
	if(!dstWidget)
		detachTab(from);
	else if(dstWidget == m_tabBar)
		moveTab(from, m_tabBar->tabAt(pos));
	else if (auto window = qobject_cast<DetachedWindow*>(dstWidget))
		moveTab(from, window);
	else if (auto tabBar = qobject_cast<DetachableTabBar*>(dstWidget))
	{
		auto tabWidget = qobject_cast<DetachableTabWidget*>(tabBar->parentWidget());
		if(tabWidget)
			moveTab(from, tabWidget);
	}
}

//****************************************************************************//

DetachedWindow::DetachedWindow(MainWindow* parent)
	: QDialog(parent)
	, m_mainWindow(parent)
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	setLayout(m_mainLayout);
	setAttribute(Qt::WA_DeleteOnClose);
	setAcceptDrops(true);
}

void DetachedWindow::attachTab(DetachableTabWidget::TabInfo tabInfo)
{
	if (!m_defaultTabContent.widget)
	{
		setWindowTitle(tabInfo.title);
		auto w = tabInfo.widget;
		w->setParent(this);
		m_mainLayout->addWidget(w);
		m_defaultTabContent = tabInfo;

		if (tabInfo.info)
			connect(tabInfo.info, &DetachableWidgetInfo::changedTitle, this, &DetachedWindow::changeTitle);
	}
	else
	{
		if (!m_tabWidget)
			createTabWidget();
		m_tabWidget->addTab(tabInfo.widget, tabInfo.title, tabInfo.info);
	}
}

bool DetachedWindow::closeTab(QWidget* tab)
{
	if (m_defaultTabContent.widget == tab)
	{
		m_defaultTabContent.widget = nullptr; // Still a child of this window, and will be deleted by it
		close();
		return true;
	}
	else
		return false;
}

void DetachedWindow::closeEvent(QCloseEvent* /*event*/)
{
	// TODO: close tabs that can be closed, move the others to the main view
	// For all tabs in tabbar, emit detachTab
	if(m_defaultTabContent.widget)
		emit detachTab(m_defaultTabContent);
	emit closedDetachedWindow(this);
}

void DetachedWindow::changeTitle(DetachableWidgetInfo*, QString title)
{
	if (!m_tabWidget)
	{
		setWindowTitle(title);
		m_defaultTabContent.title = title;
	}
}

void DetachedWindow::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if(mime->formats().contains("action") && mime->data("action") == "application/tab-detach")
		event->acceptProposedAction();

	QDialog::dragEnterEvent(event);
}

void DetachedWindow::dragMoveEvent(QDragMoveEvent* event)
{
const QMimeData* mime = event->mimeData();
	if(mime->formats().contains("action") && mime->data("action") == "application/tab-detach")
		event->acceptProposedAction();

	QDialog::dragMoveEvent(event);
}

void DetachedWindow::dropEvent(QDropEvent* event)
{
	event->setDropAction(Qt::MoveAction);
	event->accept();

	if (auto source = qobject_cast<DetachableTabBar*>(event->source()))
		source->setDropDestination(this, event->pos());
}

void DetachedWindow::createTabWidget()
{
	m_tabWidget = new DetachableTabWidget(m_mainWindow);
	connect(m_tabWidget, &DetachableTabWidget::closedTab, [this](QWidget* w) {
		emit closedTab(w);
		if (!m_tabWidget->count())
			close();
	//	if (m_tabWidget->count() == 1)
	//		removeTabWidget();
	});
	connect(m_tabWidget, &DetachableTabWidget::removedTab, [this](QWidget* w) {
		if (!m_tabWidget->count())
			close();
	//	if (m_tabWidget->count() == 1)
	//		removeTabWidget();
	});

	auto w = m_defaultTabContent.widget;
	w->disconnect(this);
	m_mainLayout->removeWidget(w);
	m_tabWidget->addTab(m_defaultTabContent.widget, m_defaultTabContent.title, m_defaultTabContent.info);
	m_mainLayout->addWidget(m_tabWidget);

	m_defaultTabContent = DetachableTabWidget::TabInfo();
	setWindowTitle(tr("Panda - detached window"));
}

void DetachedWindow::removeTabWidget()
{
	// TODO: This is not working yet
	auto info = m_tabWidget->getInfo(0);
	auto w = info.widget;
	w->setParent(this);

	m_tabWidget->removeTab(0);
	delete m_tabWidget;
	m_tabWidget = nullptr;

	attachTab(info);
}
