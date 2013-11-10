#include <QtWidgets>

#include <ui/MainWindow.h>
#include <ui/GraphView.h>
#include <ui/RenderView.h>
#include <ui/DatasTable.h>
#include <ui/EditGroupDialog.h>
#include <ui/LayersTab.h>
#include <ui/UpdateLoggerDialog.h>

#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/Group.h>

MainWindow::MainWindow()
	: groupsRegistryMenu(nullptr)
	, loggerDialog(nullptr)
{
	pandaDocument = new panda::PandaDocument(this);

	graphView = new GraphView(pandaDocument);
	renderView = new RenderView(pandaDocument);
	tabWidget = new QTabWidget;
	tabWidget->addTab(graphView, tr("Graph"));
	tabWidget->addTab(renderView, tr("Render"));
	setCentralWidget(tabWidget);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	connect(pandaDocument, SIGNAL(modified()), this, SLOT(documentModified()));
	connect(graphView, SIGNAL(modified()), this, SLOT(documentModified()));
	connect(graphView, SIGNAL(showStatusBarMessage(QString)), this, SLOT(showStatusBarMessage(QString)));
	connect(graphView, SIGNAL(showContextMenu(QPoint,int)), this, SLOT(showContextMenu(QPoint,int)));

	readSettings();

	createGroupRegistryMenu();

	setWindowIcon(QIcon(":/images/icon.png"));
	setCurrentFile("");

	datasTable = new DatasTable(pandaDocument, this);

	datasDock = new QDockWidget("Properties", this);
	datasDock->setObjectName("PropertiesDock");
	datasDock->setWidget(datasTable);
	datasDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, datasDock);

	layersTab = new LayersTab(pandaDocument, this);

	layersDock = new QDockWidget("Layers", this);
	layersDock->setObjectName("LayersDock");
	layersDock->setWidget(layersTab);
	layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, layersDock);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (okToContinue()) {
		writeSettings();
		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::newFile()
{
	if (okToContinue()) {
		playAction->setChecked(false);
		graphView->resetView();
		pandaDocument->resetDocument();
		setCurrentFile("");
	}
}

void MainWindow::open()
{
	if (okToContinue()) {
		QString fileName = QFileDialog::getOpenFileName(this,
								   tr("Open Document"), ".",
								   tr("Panda files (*.pnd)"));
		if (!fileName.isEmpty())
		{
			playAction->setChecked(false);
			pandaDocument->resetDocument();
			graphView->resetView();
			loadFile(fileName);
			graphView->updateLinkTags(true);
		}
	}
}

void MainWindow::import()
{
	QString fileName = QFileDialog::getOpenFileName(this,
							   tr("Open Document"), ".",
							   tr("Panda files (*.pnd)"));
	if (!fileName.isEmpty())
	{
		loadFile(fileName, true);
		graphView->updateLinkTags();
	}
}

bool MainWindow::save()
{
	if (curFile.isEmpty()) {
		return saveAs();
	} else {
		return saveFile(curFile);
	}
}

bool MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this,
							   tr("Save Document"), ".",
							   tr("Panda files (*.pnd)"));
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Panda"),
			tr("<h2>Panda 0.1</h2>"
			   "<p>Copyright &copy; 2012 Christophe Guébert"
			   "<p>Panda is a framework for parametric drawing and animation."));
}

void MainWindow::openRecentFile()
{
	if (okToContinue()) {
		QAction *action = qobject_cast<QAction *>(sender());
		if(action)
		{
			playAction->setChecked(false);
			pandaDocument->resetDocument();
			loadFile(action->data().toString());
			graphView->updateLinkTags(true);
		}
	}
}

void MainWindow::updateStatusBar()
{
	double time = pandaDocument->getAnimationTime();
	timeLabel->setText(tr("time : %1").arg(time));
}

void MainWindow::documentModified()
{
	if(tabWidget->currentWidget() == renderView)
		renderView->update();

	setWindowModified(true);
}

void MainWindow::createActions()
{
	newAction = new QAction(tr("&New"), this);
	newAction->setIcon(QIcon(":/images/new.png"));
	newAction->setShortcut(QKeySequence::New);
	newAction->setStatusTip(tr("Create a new panda document"));
	connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

	openAction = new QAction(tr("&Open..."), this);
	openAction->setIcon(QIcon(":/images/open.png"));
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("Open an existing panda document"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	importAction = new QAction(tr("&Import..."), this);
	importAction->setIcon(QIcon(":/images/open.png"));
	importAction->setStatusTip(tr("Import an existing panda document into the current one"));
	connect(importAction, SIGNAL(triggered()), this, SLOT(import()));

	saveAction = new QAction(tr("&Save"), this);
	saveAction->setIcon(QIcon(":/images/save.png"));
	saveAction->setShortcut(QKeySequence::Save);
	saveAction->setStatusTip(tr("Save the document to disk"));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAction = new QAction(tr("Save &As..."), this);
	saveAsAction->setStatusTip(tr("Save the document under a new "
								  "name"));
	connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	for (int i = 0; i < MaxRecentFiles; ++i) {
		recentFileActions[i] = new QAction(this);
		recentFileActions[i]->setVisible(false);
		connect(recentFileActions[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	exitAction = new QAction(tr("E&xit"), this);
	exitAction->setShortcut(tr("Ctrl+Q"));
	exitAction->setStatusTip(tr("Exit Panda"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	cutAction = new QAction(tr("Cu&t"), this);
	cutAction->setIcon(QIcon(":/images/cut.png"));
	cutAction->setShortcut(QKeySequence::Cut);
	cutAction->setStatusTip(tr("Cut the current selection's contents "
							   "to the clipboard"));
	connect(cutAction, SIGNAL(triggered()), pandaDocument, SLOT(cut()));

	copyAction = new QAction(tr("&Copy"), this);
	copyAction->setIcon(QIcon(":/images/copy.png"));
	copyAction->setShortcut(QKeySequence::Copy);
	copyAction->setStatusTip(tr("Copy the current selection's contents "
								"to the clipboard"));
	connect(copyAction, SIGNAL(triggered()), pandaDocument, SLOT(copy()));

	pasteAction = new QAction(tr("&Paste"), this);
	pasteAction->setIcon(QIcon(":/images/paste.png"));
	pasteAction->setShortcut(QKeySequence::Paste);
	pasteAction->setStatusTip(tr("Paste the clipboard's contents into "
								 "the current selection"));
	connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

	deleteAction = new QAction(tr("&Delete"), this);
	deleteAction->setShortcut(QKeySequence::Delete);
	deleteAction->setStatusTip(tr("Delete the current selection's "
								  "contents"));
	connect(deleteAction, SIGNAL(triggered()), pandaDocument, SLOT(del()));

	selectAllAction = new QAction(tr("Select &all"), this);
	selectAllAction->setShortcut(tr("Ctrl+A"));
	selectAllAction->setStatusTip(tr("Select all objects"));

	connect(selectAllAction, SIGNAL(triggered()), pandaDocument, SLOT(selectAll()));

	selectNoneAction = new QAction(tr("Select &none"), this);
	selectNoneAction->setShortcut(tr("Ctrl+Shift+A"));
	selectNoneAction->setStatusTip(tr("Deselect all objets"));

	connect(selectNoneAction, SIGNAL(triggered()), pandaDocument, SLOT(selectNone()));

	selectConnectedAction = new QAction(tr("Select &connected"), this);
	selectConnectedAction->setShortcut(tr("Ctrl+Shift+C"));
	selectConnectedAction->setStatusTip(tr("Select all objects connected to the current one"));

	connect(selectConnectedAction, SIGNAL(triggered()), pandaDocument, SLOT(selectConnected()));

	groupAction = new QAction(tr("&Group selected"), this);
	groupAction->setShortcut(tr("Ctrl+G"));
	groupAction->setStatusTip(tr("Group selected objects"));

	connect(groupAction, SIGNAL(triggered()), this, SLOT(group()));

	ungroupAction = new QAction(tr("&Ungroup selected"), this);
	ungroupAction->setShortcut(tr("Ctrl+Shift+G"));
	ungroupAction->setStatusTip(tr("Ungroup selected objects"));

	connect(ungroupAction, SIGNAL(triggered()), this, SLOT(ungroup()));

	editGroupAction = new QAction(tr("&Edit group"), this);
	editGroupAction->setShortcut(tr("Ctrl+E"));
	editGroupAction->setStatusTip(tr("Edit the selected group name and parameters"));

	connect(editGroupAction, SIGNAL(triggered()), this, SLOT(editGroup()));

	saveGroupAction = new QAction(tr("&Save group"), this);
	saveGroupAction->setShortcut(tr("Ctrl+Shift+E"));
	saveGroupAction->setStatusTip(tr("Save the selected group for later use"));

	connect(saveGroupAction, SIGNAL(triggered()), this, SLOT(saveGroup()));

	zoomResetAction = new QAction(tr("Reset &zoom"), this);
	zoomResetAction->setShortcut(tr("Ctrl+0"));
	zoomResetAction->setStatusTip(tr("Set zoom to 100%"));

	connect(zoomResetAction, SIGNAL(triggered()), graphView, SLOT(zoomReset()));

	zoomInAction = new QAction(tr("Zoom &in"), this);
	zoomInAction->setShortcut(tr("Ctrl++"));
	zoomInAction->setStatusTip(tr("Zoom in"));

	connect(zoomInAction, SIGNAL(triggered()), graphView, SLOT(zoomIn()));

	zoomOutAction = new QAction(tr("Zoom &out"), this);
	zoomOutAction->setShortcut(tr("Ctrl+-"));
	zoomOutAction->setStatusTip(tr("Zoom out"));

	connect(zoomOutAction, SIGNAL(triggered()), graphView, SLOT(zoomOut()));

	centerViewAction = new QAction(tr("&Center view"), this);
	centerViewAction->setShortcut(tr("Ctrl+5"));
	centerViewAction->setStatusTip(tr("Center the view"));

	connect(centerViewAction, SIGNAL(triggered()), graphView, SLOT(centerView()));

	showAllAction = new QAction(tr("Show &all"), this);
	showAllAction->setShortcut(tr("Ctrl+f"));
	showAllAction->setStatusTip(tr("Center and zoom the view so that all objects are visible"));

	connect(showAllAction, SIGNAL(triggered()), graphView, SLOT(showAll()));

	showAllSelectedAction = new QAction(tr("Show all &selected"), this);
	showAllSelectedAction->setShortcut(tr("Ctrl+d"));
	showAllSelectedAction->setStatusTip(tr("Center and zoom the view so that all selected objects are visible"));

	connect(showAllSelectedAction, SIGNAL(triggered()), graphView, SLOT(showAllSelected()));

	showGraphView = new QAction(tr("Show &graph view"), this);
	showGraphView->setShortcut(tr("Ctrl+1"));
	showGraphView->setStatusTip(tr("Switch to the graph view"));

	connect(showGraphView, SIGNAL(triggered()), this, SLOT(switchToGraphView()));

	showRenderView = new QAction(tr("Show &render view"), this);
	showRenderView->setShortcut(tr("Ctrl+2"));
	showRenderView->setStatusTip(tr("Switch to the render view"));

	connect(showRenderView, SIGNAL(triggered()), this, SLOT(switchToRenderView()));

	aboutAction = new QAction(tr("&About"), this);
	aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAction = new QAction(tr("About &Qt"), this);
	aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	playAction = new QAction(tr("Play"), this);
	playAction->setIcon(QIcon(":/images/play.png"));
	playAction->setShortcut(tr("F5"));
	playAction->setStatusTip(tr("Start the animation"));
	playAction->setCheckable(true);
	connect(playAction, SIGNAL(triggered(bool)), pandaDocument, SLOT(play(bool)));

	stepAction = new QAction(tr("Step"), this);
	stepAction->setIcon(QIcon(":/images/step.png"));
	stepAction->setShortcut(tr("F6"));
	stepAction->setStatusTip(tr("Do one step of the animation"));
	connect(stepAction, SIGNAL(triggered()), pandaDocument, SLOT(step()));

	rewindAction = new QAction(tr("Rewind"), this);
	rewindAction->setIcon(QIcon(":/images/stop.png"));
	rewindAction->setShortcut(tr("F7"));
	rewindAction->setStatusTip(tr("Rewind the animation back to the begining"));
	connect(rewindAction, SIGNAL(triggered()), pandaDocument, SLOT(rewind()));

	removeLinkAction = new QAction(tr("Remove link"), this);
	removeLinkAction->setStatusTip(tr("Remove the link to this data"));
	connect(removeLinkAction, SIGNAL(triggered()), graphView, SLOT(removeLink()));

	copyDataAction = new QAction(tr("Copy data"), this);
	copyDataAction->setStatusTip(tr("Create a user value generator based on this data"));
	connect(copyDataAction, SIGNAL(triggered()), this, SLOT(copyDataToUserValue()));

	showLoggerDialogAction = new QAction(tr("Show log"), this);
	showLoggerDialogAction->setStatusTip(tr("Show the updates log dialog"));
	connect(showLoggerDialogAction, SIGNAL(triggered()), this, SLOT(showLoggerDialog()));
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAction);
	fileMenu->addAction(openAction);
	fileMenu->addAction(importAction);
	fileMenu->addAction(saveAction);
	fileMenu->addAction(saveAsAction);
	separatorAction = fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		fileMenu->addAction(recentFileActions[i]);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(cutAction);
	editMenu->addAction(copyAction);
	editMenu->addAction(pasteAction);
	editMenu->addAction(deleteAction);

	selectMenu = editMenu->addMenu(tr("&Select"));
	selectMenu->addAction(selectAllAction);
	selectMenu->addAction(selectNoneAction);
	selectMenu->addAction(selectConnectedAction);

	groupMenu = editMenu->addMenu(tr("&Group"));
	groupMenu->addAction(groupAction);
	groupMenu->addAction(ungroupAction);
	groupMenu->addAction(editGroupAction);
	groupMenu->addAction(saveGroupAction);

	createRegistryMenu();

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(zoomInAction);
	viewMenu->addAction(zoomOutAction);
	viewMenu->addAction(zoomResetAction);
	viewMenu->addSeparator();
	viewMenu->addAction(centerViewAction);
	viewMenu->addAction(showAllAction);
	viewMenu->addAction(showAllSelectedAction);
	viewMenu->addSeparator();
	viewMenu->addAction(showGraphView);
	viewMenu->addAction(showRenderView);
#ifdef PANDA_LOG_EVENTS
	viewMenu->addSeparator();
	viewMenu->addAction(showLoggerDialogAction);
#endif

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAction);
	helpMenu->addAction(aboutQtAction);
}

struct menuItemInfo
{
	typedef QMap<QString, QAction*> ActionsMap;
	typedef QMap<QString, menuItemInfo> ChildsMap;

	ActionsMap actions;
	ChildsMap childs;

	void registerActions(QMenu* menu)
	{
		for(ActionsMap::iterator it=actions.begin(); it!=actions.end(); ++it)
			menu->addAction(it.value());

		for(ChildsMap::iterator it=childs.begin(); it!=childs.end(); ++it)
			it.value().registerActions(menu->addMenu(it.key()));
	}
};

void MainWindow::createRegistryMenu()
{
	panda::ObjectFactory::RegistryMapIterator iter = panda::ObjectFactory::getInstance()->getRegistryIterator();
	if(iter.hasNext())
	{
		registryMenu = menuBar()->addMenu(tr("&Add"));

		menuItemInfo menuTree;

		while(iter.hasNext())
		{
			iter.next();
			const panda::ObjectFactory::ClassEntry* entry = iter.value().data();
			if(entry->hidden)
				continue;

			QString display = entry->menuDisplay;
			QStringList hierarchy = display.split("/");
			menuItemInfo* currentMenu = &menuTree;
			for(int i=0; i<hierarchy.count()-1; ++i)
				currentMenu = &currentMenu->childs[hierarchy[i]];

			QAction* tempAction = new QAction(hierarchy.last(), this);
			if(entry)
				tempAction->setStatusTip(entry->description);
			tempAction->setData(entry->className);
			currentMenu->actions[hierarchy.last()] = tempAction;

			connect(tempAction, SIGNAL(triggered()), this, SLOT(createObject()));
		}

		menuTree.registerActions(registryMenu);
	}
}

void MainWindow::createGroupRegistryMenu()
{
	if(groupsRegistryMenu)
		groupsRegistryMenu->clear();
	pandaDocument->createGroupsList();

	panda::PandaDocument::GroupsIterator iter = pandaDocument->getGroupsIterator();
	if(iter.hasNext())
	{
		if(!groupsRegistryMenu)
		{
			registryMenu->addSeparator();
			groupsRegistryMenu = registryMenu->addMenu(tr("&Groups"));
		}

		menuItemInfo menuTree;
		while(iter.hasNext())
		{
			iter.next();
			QString display = iter.key();
			QStringList hierarchy = display.split("/");
			menuItemInfo* currentMenu = &menuTree;
			for(int i=0; i<hierarchy.count()-1; ++i)
				currentMenu = &currentMenu->childs[hierarchy[i]];

			QAction* tempAction = new QAction(hierarchy.last(), this);
			tempAction->setStatusTip(iter.value());
			tempAction->setData(iter.key());
			currentMenu->actions[hierarchy.last()] = tempAction;

			connect(tempAction, SIGNAL(triggered()), this, SLOT(createGroupObject()));
		}

		menuTree.registerActions(groupsRegistryMenu);
	}
}

void MainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("&File"));
	fileToolBar->setObjectName("FileToolBar");
	fileToolBar->addAction(newAction);
	fileToolBar->addAction(openAction);
	fileToolBar->addAction(saveAction);

	editToolBar = addToolBar(tr("&Edit"));
	editToolBar->setObjectName("EditToolBar");
	editToolBar->addAction(cutAction);
	editToolBar->addAction(copyAction);
	editToolBar->addAction(pasteAction);

	animToolBar = addToolBar(tr("&Animation"));
	animToolBar->setObjectName("AnimToolBar");
	animToolBar->addAction(playAction);
	animToolBar->addAction(stepAction);
	animToolBar->addAction(rewindAction);
}

void MainWindow::createStatusBar()
{
	timeLabel = new QLabel(tr("time : %1").arg(999.99));
	timeLabel->setAlignment(Qt::AlignLeft);
	timeLabel->setMinimumSize(timeLabel->sizeHint());

	statusBar()->addWidget(timeLabel);

	connect(pandaDocument, SIGNAL(timeChanged()), this, SLOT(updateStatusBar()));

	updateStatusBar();
}

void MainWindow::readSettings()
{
	QSettings settings("Christophe Guebert", "Panda");

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());

	recentFiles = settings.value("recentFiles").toStringList();
	updateRecentFileActions();
}

void MainWindow::writeSettings()
{
	QSettings settings("Christophe Guebert", "Panda");

	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("recentFiles", recentFiles);
}

bool MainWindow::okToContinue()
{
	if (isWindowModified()) {
		int r = QMessageBox::warning(this, tr("Panda"),
						tr("The document has been modified.\n"
						   "Do you want to save your changes?"),
						QMessageBox::Yes | QMessageBox::No
						| QMessageBox::Cancel);
		if (r == QMessageBox::Yes) {
			return save();
		} else if (r == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}

bool MainWindow::loadFile(const QString &fileName, bool import)
{
	if (!pandaDocument->readFile(fileName)) {
		statusBar()->showMessage(tr("Loading failed"), 2000);
		return false;
	}

	if(!import)
	{
		setCurrentFile(fileName);
		statusBar()->showMessage(tr("File loaded"), 2000);
	}
	else
		statusBar()->showMessage(tr("File imported"), 2000);
	graphView->showAll();
	return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
	if (!pandaDocument->writeFile(fileName)) {
		statusBar()->showMessage(tr("Saving failed"), 2000);
		return false;
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File saved"), 2000);
	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	curFile = fileName;
	setWindowModified(false);

	QString shownName = tr("Untitled");
	if (!curFile.isEmpty()) {
		shownName = strippedName(curFile);
		recentFiles.removeAll(curFile);
		recentFiles.prepend(curFile);
		updateRecentFileActions();
	}

	setWindowTitle(tr("%1[*] - %2").arg(shownName)
								   .arg(tr("Panda")));
}

void MainWindow::updateRecentFileActions()
{
	QMutableStringListIterator i(recentFiles);
	while (i.hasNext()) {
		if (!QFile::exists(i.next()))
			i.remove();
	}

	for (int j = 0; j < MaxRecentFiles; ++j) {
		if (j < recentFiles.count()) {
			QString text = tr("&%1 %2")
						   .arg(j + 1)
						   .arg(strippedName(recentFiles[j]));
			recentFileActions[j]->setText(text);
			recentFileActions[j]->setData(recentFiles[j]);
			recentFileActions[j]->setVisible(true);
		} else {
			recentFileActions[j]->setVisible(false);
		}
	}
	separatorAction->setVisible(!recentFiles.isEmpty());
}

QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::createObject()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if(action)
		pandaDocument->createObject(action->data().toString());
}

void MainWindow::switchToGraphView()
{
	tabWidget->setCurrentWidget(graphView);
}

void MainWindow::switchToRenderView()
{
	tabWidget->setCurrentWidget(renderView);
}

void MainWindow::showStatusBarMessage(QString text)
{
	statusBar()->showMessage(text, 2000);
}

void MainWindow::paste()
{
	pandaDocument->paste();
	graphView->moveSelectedToCenter();
	graphView->updateLinkTags();
}

void MainWindow::group()
{
	bool res = panda::Group::createGroup(pandaDocument, graphView);
	if(!res)
		statusBar()->showMessage(tr("Could not create a group from the selection"), 2000);
}

void MainWindow::ungroup()
{
	bool res = panda::Group::ungroupSelection(pandaDocument, graphView);
	if(!res)
		statusBar()->showMessage(tr("Could not ungroup the selection"), 2000);
}

void MainWindow::editGroup()
{
	panda::Group* group = dynamic_cast<panda::Group*>(pandaDocument->getCurrentSelectedObject());
	if(group)
	{
		EditGroupDialog dlg(group, this);
		dlg.exec();
	}
	else
		statusBar()->showMessage(tr("The selected objet is not a group"), 2000);
}

void MainWindow::saveGroup()
{
	panda::PandaObject* object = pandaDocument->getCurrentSelectedObject();
	panda::Group* group = dynamic_cast<panda::Group*>(object);
	if(group)
	{
		if(pandaDocument->saveGroup(group))
		{
			statusBar()->showMessage(tr("Group saved"), 2000);
			createGroupRegistryMenu();
		}
	}
	else
		statusBar()->showMessage(tr("The selected objet is not a group"), 2000);
}


void MainWindow::createGroupObject()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if(action)
	{
		QString path = action->data().toString();
		pandaDocument->createGroupObject(path);
	}
}

void MainWindow::showContextMenu(QPoint pos, int flags)
{
	QMenu menu(this);

	panda::PandaObject* obj = pandaDocument->getCurrentSelectedObject();
	if(obj)
	{
		menu.addAction(cutAction);
		menu.addAction(copyAction);
	}
	menu.addAction(pasteAction);

	if(flags & GraphView::MENU_LINK)
	{
		menu.addAction(removeLinkAction);
	}

	const panda::BaseData* clickedData = graphView->getContextMenuData();
	if(clickedData && clickedData->isDisplayed())
		menu.addAction(copyDataAction);

	int nbSelected = pandaDocument->getSelection().size();
	if(dynamic_cast<panda::Group*>(obj) && nbSelected == 1)
	{
		menu.addAction(ungroupAction);
		menu.addAction(editGroupAction);
		menu.addAction(saveGroupAction);
	}

	if(nbSelected > 1)
	{
		menu.addAction(groupAction);
	}

	if(!menu.actions().isEmpty())
		menu.exec(pos);
}

void MainWindow::copyDataToUserValue()
{
	const panda::BaseData* clickedData = graphView->getContextMenuData();
	if(clickedData)
		pandaDocument->copyDataToUserValue(clickedData);
}

void MainWindow::showLoggerDialog()
{
	if(!loggerDialog)
	{
		loggerDialog = new UpdateLoggerDialog(this);
		UpdateLoggerDialog::setInstance(loggerDialog);

		connect(loggerDialog, SIGNAL(changedSelectedEvent()), graphView, SLOT(update()));
	}

	if(loggerDialog->isVisible())
		loggerDialog->hide();
	else
	{
		loggerDialog->updateEvents();
		loggerDialog->show();
	}
}
