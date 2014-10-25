#include <QtWidgets>

#include <ui/CreateGroup.h>
#include <ui/DatasTable.h>
#include <ui/EditGroupDialog.h>
#include <ui/GraphView.h>
#include <ui/GroupsManager.h>
#include <ui/OpenGLRenderView.h>
#include <ui/PluginsManager.h>
#include <ui/LayersTab.h>
#include <ui/ListObjectsAndTypes.h>
#include <ui/MainWindow.h>
#include <ui/UpdateLoggerDialog.h>

#include <ui/command/AddObjectCommand.h>
#include <ui/command/RemoveObjectCommand.h>

#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/Group.h>
#include <panda/helper/system/FileRepository.h>

MainWindow::MainWindow()
	: m_groupsRegistryMenu(nullptr)
	, m_loggerDialog(nullptr)
	, m_fullScreen(false)
	, m_adjustRenderSizeToView(false)
{
	m_document = new panda::PandaDocument(this);

	m_graphView = new GraphView(m_document);
	m_openGLRenderView = new OpenGLRenderView(m_document);
	m_tabWidget = new QTabWidget;
	m_tabWidget->addTab(m_graphView, tr("Graph"));
	m_tabWidget->addTab(m_openGLRenderView, tr("Render"));
	setCentralWidget(m_tabWidget);

	// Set the application directories
	QStringList standardPaths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	for(const QString& path : standardPaths)
		panda::helper::system::DataRepository.addPath(path);

	PluginsManager::getInstance()->loadPlugins();

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	connect(m_document, SIGNAL(modified()), this, SLOT(documentModified()));
	connect(m_document, SIGNAL(selectedObject(panda::PandaObject*)), this, SLOT(selectedObject(panda::PandaObject*)));
	connect(m_graphView, SIGNAL(modified()), this, SLOT(documentModified()));
	connect(m_graphView, SIGNAL(showStatusBarMessage(QString)), this, SLOT(showStatusBarMessage(QString)));
	connect(m_graphView, SIGNAL(showContextMenu(QPoint,int)), this, SLOT(showContextMenu(QPoint,int)));

	createGroupRegistryMenu();

	setWindowIcon(QIcon(":/share/icons/icon.png"));
	setCurrentFile("");

	m_datasTable = new DatasTable(m_document, this);

	m_datasDock = new QDockWidget(tr("Properties"), this);
	m_datasDock->setObjectName("PropertiesDock");
	m_datasDock->setWidget(m_datasTable);
	m_datasDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_datasDock);

	m_layersTab = new LayersTab(m_document, this);

	m_layersDock = new QDockWidget(tr("Layers"), this);
	m_layersDock->setObjectName("LayersDock");
	m_layersDock->setWidget(m_layersTab);
	m_layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_layersDock);

	readSettings();
}

void MainWindow::closeEvent(QCloseEvent* event)
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
		m_playAction->setChecked(false);
		play(false);
		m_graphView->resetView();
		m_document->resetDocument();
		setCurrentFile("");
	}
}

void MainWindow::open()
{
	if (okToContinue()) {
		QString fileName = QFileDialog::getOpenFileName(this,
								   tr("Open Document"), ".",
								   tr("Panda files (*.pnd);;XML Files (*.xml)"));
		if (!fileName.isEmpty())
		{
			m_playAction->setChecked(false);
			m_document->resetDocument();
			m_graphView->resetView();
			loadFile(fileName);
			m_graphView->updateLinkTags(true);
		}
	}
}

void MainWindow::import()
{
	QString fileName = QFileDialog::getOpenFileName(this,
							   tr("Open Document"), ".",
							   tr("Panda files (*.pnd);;XML Files (*.xml)"));
	if (!fileName.isEmpty())
	{
		if(loadFile(fileName, true))
		{
			m_graphView->updateLinkTags();

			auto selection = m_document->getSelection();
			if(!selection.empty())
				m_document->addCommand(new AddObjectCommand(m_document, m_graphView, selection));
		}
	}
}

bool MainWindow::save()
{
	if (m_curFile.isEmpty()) {
		return saveAs();
	} else {
		return saveFile(m_curFile);
	}
}

bool MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this,
							   tr("Save Document"), ".",
							   tr("Panda files (*.pnd);;XML Files (*.xml)"));
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Panda"),
			tr("<h2>Panda 0.2</h2>"
			   "<p>Copyright &copy; 2014 Christophe Guébert"
			   "<p>Panda is a framework for parametric drawing and animation."));
}

void MainWindow::openRecentFile()
{
	if (okToContinue()) {
		QAction *action = qobject_cast<QAction *>(sender());
		if(action)
		{
			m_playAction->setChecked(false);
			m_document->resetDocument();
			loadFile(action->data().toString());
			m_graphView->updateLinkTags(true);
		}
	}
}

void MainWindow::updateStatusBar()
{
	PReal time = m_document->getAnimationTime();
	PReal FPS = m_document->getFPS();
	m_timeLabel->setText(tr("time: %1\tFPS: %2").arg(time).arg(QString::number(FPS, 'f', 1)));
}

void MainWindow::documentModified()
{
	if(m_tabWidget->currentWidget() == m_openGLRenderView)
		m_openGLRenderView->update();

	setWindowModified(true);
}

void MainWindow::createActions()
{
	m_newAction = new QAction(tr("&New"), this);
	m_newAction->setIcon(QIcon(":/share/icons/new.png"));
	m_newAction->setShortcut(QKeySequence::New);
	m_newAction->setStatusTip(tr("Create a new panda document"));
	connect(m_newAction, SIGNAL(triggered()), this, SLOT(newFile()));

	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(":/share/icons/open.png"));
	m_openAction->setShortcut(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open an existing panda document"));
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_importAction = new QAction(tr("&Import..."), this);
	m_importAction->setIcon(QIcon(":/share/icons/open.png"));
	m_importAction->setStatusTip(tr("Import an existing panda document into the current one"));
	connect(m_importAction, SIGNAL(triggered()), this, SLOT(import()));

	m_saveAction = new QAction(tr("&Save"), this);
	m_saveAction->setIcon(QIcon(":/share/icons/save.png"));
	m_saveAction->setShortcut(QKeySequence::Save);
	m_saveAction->setStatusTip(tr("Save the document to disk"));
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));

	m_saveAsAction = new QAction(tr("Save &As..."), this);
	m_saveAsAction->setStatusTip(tr("Save the document under a new "
								  "name"));
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	m_exitAction = new QAction(tr("E&xit"), this);
	m_exitAction->setShortcut(tr("Ctrl+Q"));
	m_exitAction->setStatusTip(tr("Exit Panda"));
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	m_cutAction = new QAction(tr("Cu&t"), this);
	m_cutAction->setIcon(QIcon(":/share/icons/cut.png"));
	m_cutAction->setShortcut(QKeySequence::Cut);
	m_cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	connect(m_cutAction, SIGNAL(triggered()), this, SLOT(cut()));
	addAction(m_cutAction);

	m_copyAction = new QAction(tr("&Copy"), this);
	m_copyAction->setIcon(QIcon(":/share/icons/copy.png"));
	m_copyAction->setShortcut(QKeySequence::Copy);
	m_copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	connect(m_copyAction, SIGNAL(triggered()), m_document, SLOT(copy()));
	addAction(m_copyAction);

	m_pasteAction = new QAction(tr("&Paste"), this);
	m_pasteAction->setIcon(QIcon(":/share/icons/paste.png"));
	m_pasteAction->setShortcut(QKeySequence::Paste);
	m_pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	connect(m_pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
	addAction(m_pasteAction);

	m_deleteAction = new QAction(tr("&Delete"), this);
	m_deleteAction->setShortcut(QKeySequence::Delete);
	m_deleteAction->setStatusTip(tr("Delete the current selection's contents"));
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(del()));
	addAction(m_deleteAction);

	m_selectAllAction = new QAction(tr("Select &all"), this);
	m_selectAllAction->setShortcut(tr("Ctrl+A"));
	m_selectAllAction->setStatusTip(tr("Select all objects"));
	connect(m_selectAllAction, SIGNAL(triggered()), m_document, SLOT(selectAll()));
	addAction(m_selectAllAction);

	m_selectNoneAction = new QAction(tr("Select &none"), this);
	m_selectNoneAction->setShortcut(tr("Ctrl+Shift+A"));
	m_selectNoneAction->setStatusTip(tr("Deselect all objets"));
	connect(m_selectNoneAction, SIGNAL(triggered()), m_document, SLOT(selectNone()));
	addAction(m_selectNoneAction);

	m_selectConnectedAction = new QAction(tr("Select &connected"), this);
	m_selectConnectedAction->setShortcut(tr("Ctrl+Shift+C"));
	m_selectConnectedAction->setStatusTip(tr("Select all objects connected to the current one"));
	connect(m_selectConnectedAction, SIGNAL(triggered()), m_document, SLOT(selectConnected()));
	addAction(m_selectConnectedAction);

	m_groupAction = new QAction(tr("&Group selected"), this);
	m_groupAction->setShortcut(tr("Ctrl+G"));
	m_groupAction->setStatusTip(tr("Group selected objects"));
	m_groupAction->setEnabled(false);
	connect(m_groupAction, SIGNAL(triggered()), this, SLOT(group()));
	addAction(m_groupAction);

	m_ungroupAction = new QAction(tr("&Ungroup selected"), this);
	m_ungroupAction->setShortcut(tr("Ctrl+Shift+G"));
	m_ungroupAction->setStatusTip(tr("Ungroup selected objects"));
	m_ungroupAction->setEnabled(false);
	connect(m_ungroupAction, SIGNAL(triggered()), this, SLOT(ungroup()));
	addAction(m_ungroupAction);

	m_editGroupAction = new QAction(tr("&Edit group"), this);
	m_editGroupAction->setShortcut(tr("Ctrl+E"));
	m_editGroupAction->setStatusTip(tr("Edit the selected group name and parameters"));
	m_editGroupAction->setEnabled(false);
	connect(m_editGroupAction, SIGNAL(triggered()), this, SLOT(editGroup()));
	addAction(m_editGroupAction);

	m_saveGroupAction = new QAction(tr("&Save group"), this);
	m_saveGroupAction->setShortcut(tr("Ctrl+Shift+E"));
	m_saveGroupAction->setStatusTip(tr("Save the selected group for later use"));
	m_saveGroupAction->setEnabled(false);
	connect(m_saveGroupAction, SIGNAL(triggered()), this, SLOT(saveGroup()));
	addAction(m_saveGroupAction);

	m_zoomResetAction = new QAction(tr("Reset &zoom"), this);
	m_zoomResetAction->setShortcut(tr("Ctrl+0"));
	m_zoomResetAction->setStatusTip(tr("Set zoom to 100%"));
	connect(m_zoomResetAction, SIGNAL(triggered()), m_graphView, SLOT(zoomReset()));
	addAction(m_zoomResetAction);

	m_zoomInAction = new QAction(tr("Zoom &in"), this);
	m_zoomInAction->setShortcut(tr("Ctrl++"));
	m_zoomInAction->setStatusTip(tr("Zoom in"));
	connect(m_zoomInAction, SIGNAL(triggered()), m_graphView, SLOT(zoomIn()));
	addAction(m_zoomInAction);

	m_zoomOutAction = new QAction(tr("Zoom &out"), this);
	m_zoomOutAction->setShortcut(tr("Ctrl+-"));
	m_zoomOutAction->setStatusTip(tr("Zoom out"));
	connect(m_zoomOutAction, SIGNAL(triggered()), m_graphView, SLOT(zoomOut()));
	addAction(m_zoomOutAction);

	m_centerViewAction = new QAction(tr("&Center view"), this);
	m_centerViewAction->setShortcut(tr("Ctrl+5"));
	m_centerViewAction->setStatusTip(tr("Center the view"));
	connect(m_centerViewAction, SIGNAL(triggered()), m_graphView, SLOT(centerView()));
	addAction(m_centerViewAction);

	m_showAllAction = new QAction(tr("Show &all"), this);
	m_showAllAction->setShortcut(tr("Ctrl+f"));
	m_showAllAction->setStatusTip(tr("Center and zoom the view so that all objects are visible"));
	connect(m_showAllAction, SIGNAL(triggered()), m_graphView, SLOT(showAll()));
	addAction(m_showAllAction);

	m_showAllSelectedAction = new QAction(tr("Show all &selected"), this);
	m_showAllSelectedAction->setShortcut(tr("Ctrl+d"));
	m_showAllSelectedAction->setStatusTip(tr("Center and zoom the view so that all selected objects are visible"));
	connect(m_showAllSelectedAction, SIGNAL(triggered()), m_graphView, SLOT(showAllSelected()));
	addAction(m_showAllSelectedAction);

	m_showGraphViewAction = new QAction(tr("Show &graph view"), this);
	m_showGraphViewAction->setShortcut(tr("Ctrl+1"));
	m_showGraphViewAction->setStatusTip(tr("Switch to the graph view"));
	connect(m_showGraphViewAction, SIGNAL(triggered()), this, SLOT(switchToGraphView()));
	addAction(m_showGraphViewAction);

	m_showOpenGLViewAction = new QAction(tr("Show &render view"), this);
	m_showOpenGLViewAction->setShortcut(tr("Ctrl+2"));
	m_showOpenGLViewAction->setStatusTip(tr("Switch to the render view"));
	connect(m_showOpenGLViewAction, SIGNAL(triggered()), this, SLOT(switchToOpenGLView()));
	addAction(m_showOpenGLViewAction);

	m_adjustRenderSizeToViewAction = new QAction(tr("Adjust size to &view"), this);
	m_adjustRenderSizeToViewAction->setShortcut(tr("F10"));
	m_adjustRenderSizeToViewAction->setStatusTip(tr("Adjust the render size to the view"));
	m_adjustRenderSizeToViewAction->setCheckable(true);
	m_adjustRenderSizeToViewAction->setChecked(false);
	connect(m_adjustRenderSizeToViewAction, SIGNAL(triggered()), this, SLOT(adjustRenderSizeToView()));
	addAction(m_adjustRenderSizeToViewAction);

	m_fullScreenAction = new QAction(tr("&Full screen"), this);
	m_fullScreenAction->setShortcut(tr("F11"));
	m_fullScreenAction->setStatusTip(tr("Put the application in full screen mode"));
	m_fullScreenAction->setCheckable(true);
	m_fullScreenAction->setChecked(false);
	connect(m_fullScreenAction, SIGNAL(triggered()), this, SLOT(switchFullScreen()));
	addAction(m_fullScreenAction);

	m_aboutAction = new QAction(tr("&About"), this);
	m_aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	m_aboutQtAction = new QAction(tr("About &Qt"), this);
	m_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(m_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	m_playAction = new QAction(tr("Play"), this);
	m_playAction->setIcon(QIcon(":/share/icons/play.png"));
	m_playAction->setShortcut(tr("F5"));
	m_playAction->setStatusTip(tr("Start the animation"));
	m_playAction->setCheckable(true);
	connect(m_playAction, SIGNAL(triggered(bool)), m_document, SLOT(play(bool)));
	connect(m_playAction, SIGNAL(triggered(bool)), this, SLOT(play(bool)));
	addAction(m_playAction);

	m_stepAction = new QAction(tr("Step"), this);
	m_stepAction->setIcon(QIcon(":/share/icons/step.png"));
	m_stepAction->setShortcut(tr("F6"));
	m_stepAction->setStatusTip(tr("Do one step of the animation"));
	connect(m_stepAction, SIGNAL(triggered()), m_document, SLOT(step()));
	addAction(m_stepAction);

	m_rewindAction = new QAction(tr("Rewind"), this);
	m_rewindAction->setIcon(QIcon(":/share/icons/stop.png"));
	m_rewindAction->setShortcut(tr("F7"));
	m_rewindAction->setStatusTip(tr("Rewind the animation back to the begining"));
	connect(m_rewindAction, SIGNAL(triggered()), m_document, SLOT(rewind()));
	addAction(m_rewindAction);

	m_removeLinkAction = new QAction(tr("Remove link"), this);
	m_removeLinkAction->setStatusTip(tr("Remove the link to this data"));
	connect(m_removeLinkAction, SIGNAL(triggered()), m_graphView, SLOT(removeLink()));
	addAction(m_removeLinkAction);

	m_copyDataAction = new QAction(tr("Copy data"), this);
	m_copyDataAction->setStatusTip(tr("Create a user value generator based on this data"));
	connect(m_copyDataAction, SIGNAL(triggered()), this, SLOT(copyDataToUserValue()));
	addAction(m_copyDataAction);

	m_showLoggerDialogAction = new QAction(tr("Show &log"), this);
	m_showLoggerDialogAction->setStatusTip(tr("Show the updates log dialog"));
	connect(m_showLoggerDialogAction, SIGNAL(triggered()), this, SLOT(showLoggerDialog()));

	m_showObjectsAndTypesAction = new QAction(tr("List types and objects"), this);
	m_showObjectsAndTypesAction->setStatusTip(tr("Show information about all available types and objects"));
	connect(m_showObjectsAndTypesAction, SIGNAL(triggered()), this, SLOT(showObjectsAndTypes()));

	m_document->createUndoRedoActions(this, m_undoAction, m_redoAction);
	m_undoAction->setShortcut(QKeySequence::Undo);
	m_redoAction->setShortcut(QKeySequence::Redo);
	addAction(m_undoAction);
	addAction(m_redoAction);
}

void MainWindow::createMenus()
{
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_newAction);
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_importAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_separatorAction = m_fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		m_fileMenu->addAction(m_recentFileActions[i]);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	m_editMenu = menuBar()->addMenu(tr("&Edit"));
	m_editMenu->addAction(m_undoAction);
	m_editMenu->addAction(m_redoAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_cutAction);
	m_editMenu->addAction(m_copyAction);
	m_editMenu->addAction(m_pasteAction);
	m_editMenu->addAction(m_deleteAction);
	m_editMenu->addSeparator();

	m_selectMenu = m_editMenu->addMenu(tr("&Select"));
	m_selectMenu->addAction(m_selectAllAction);
	m_selectMenu->addAction(m_selectNoneAction);
	m_selectMenu->addAction(m_selectConnectedAction);

	m_groupMenu = m_editMenu->addMenu(tr("&Group"));
	m_groupMenu->addAction(m_groupAction);
	m_groupMenu->addAction(m_ungroupAction);
	m_groupMenu->addAction(m_editGroupAction);
	m_groupMenu->addAction(m_saveGroupAction);

	createRegistryMenu();

	m_viewMenu = menuBar()->addMenu(tr("&View"));
	m_viewMenu->addAction(m_zoomInAction);
	m_viewMenu->addAction(m_zoomOutAction);
	m_viewMenu->addAction(m_zoomResetAction);
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(m_centerViewAction);
	m_viewMenu->addAction(m_showAllAction);
	m_viewMenu->addAction(m_showAllSelectedAction);
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(m_showGraphViewAction);
	m_viewMenu->addAction(m_showOpenGLViewAction);
	m_viewMenu->addAction(m_adjustRenderSizeToViewAction);
	m_viewMenu->addAction(m_fullScreenAction);
#ifdef PANDA_LOG_EVENTS
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(m_showLoggerDialogAction);
#endif

	menuBar()->addSeparator();

	m_helpMenu = menuBar()->addMenu(tr("&Help"));
	m_helpMenu->addAction(m_showObjectsAndTypesAction);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_aboutAction);
	m_helpMenu->addAction(m_aboutQtAction);
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
	const auto& registryMap = panda::ObjectFactory::getInstance()->getRegistryMap();
	if(!registryMap.empty())
	{
		m_registryMenu = menuBar()->addMenu(tr("&Add"));

		menuItemInfo menuTree;

		for(const auto& iter : registryMap)
		{
			const panda::ObjectFactory::ClassEntry& entry = iter.second;
			if(entry.hidden)
				continue;

			QString display = entry.menuDisplay;
			QStringList hierarchy = display.split("/");
			menuItemInfo* currentMenu = &menuTree;
			for(int i=0; i<hierarchy.count()-1; ++i)
				currentMenu = &currentMenu->childs[hierarchy[i]];

			QAction* tempAction = new QAction(hierarchy.last(), this);
			if(!entry.description.isEmpty())
				tempAction->setStatusTip(entry.description);
			tempAction->setData(entry.className);
			currentMenu->actions[hierarchy.last()] = tempAction;

			connect(tempAction, SIGNAL(triggered()), this, SLOT(createObject()));
		}

		menuTree.registerActions(m_registryMenu);
	}
}

void MainWindow::createGroupRegistryMenu()
{
	if(m_groupsRegistryMenu)
		m_groupsRegistryMenu->clear();
	GroupsManager::getInstance()->createGroupsList();

	const auto& groups = GroupsManager::getInstance()->getGroups();
	if(!groups.empty())
	{
		if(!m_groupsRegistryMenu)
		{
			m_registryMenu->addSeparator();
			m_groupsRegistryMenu = m_registryMenu->addMenu(tr("&Groups"));
		}

		menuItemInfo menuTree;
		for(const auto& group : groups)
		{
			const QString& display = group.first;
			QStringList hierarchy = display.split("/");
			menuItemInfo* currentMenu = &menuTree;
			for(int i=0; i<hierarchy.count()-1; ++i)
				currentMenu = &currentMenu->childs[hierarchy[i]];

			QAction* tempAction = new QAction(hierarchy.last(), this);
			tempAction->setStatusTip(group.second);
			tempAction->setData(group.first);
			currentMenu->actions[hierarchy.last()] = tempAction;

			connect(tempAction, SIGNAL(triggered()), this, SLOT(createGroupObject()));
		}

		menuTree.registerActions(m_groupsRegistryMenu);
	}
}

void MainWindow::createToolBars()
{
	m_fileToolBar = addToolBar(tr("&File"));
	m_fileToolBar->setObjectName("FileToolBar");
	m_fileToolBar->addAction(m_newAction);
	m_fileToolBar->addAction(m_openAction);
	m_fileToolBar->addAction(m_saveAction);

	m_editToolBar = addToolBar(tr("&Edit"));
	m_editToolBar->setObjectName("EditToolBar");
	m_editToolBar->addAction(m_cutAction);
	m_editToolBar->addAction(m_copyAction);
	m_editToolBar->addAction(m_pasteAction);

	m_animToolBar = addToolBar(tr("&Animation"));
	m_animToolBar->setObjectName("AnimToolBar");
	m_animToolBar->addAction(m_playAction);
	m_animToolBar->addAction(m_stepAction);
	m_animToolBar->addAction(m_rewindAction);
}

void MainWindow::createStatusBar()
{
	m_timeLabel = new QLabel(tr("time: %1\t FPS: %2").arg(999.99).arg(999.9));
	m_timeLabel->setAlignment(Qt::AlignLeft);
	m_timeLabel->setMinimumSize(m_timeLabel->sizeHint());

	statusBar()->addWidget(m_timeLabel);

	connect(m_document, SIGNAL(timeChanged()), this, SLOT(updateStatusBar()));

	updateStatusBar();
}

void MainWindow::readSettings()
{
	QSettings settings("Christophe Guebert", "Panda");

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());

	m_recentFiles = settings.value("recentFiles").toStringList();
	updateRecentFileActions();
}

void MainWindow::writeSettings()
{
	QSettings settings("Christophe Guebert", "Panda");

	settings.setValue("recentFiles", m_recentFiles);

	if(!m_fullScreen)
	{
		settings.setValue("geometry", saveGeometry());
		settings.setValue("state", saveState());
	}
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
	if (!m_document->readFile(fileName, import)) {
		statusBar()->showMessage(tr("Loading failed"), 2000);
		return false;
	}

	if(!import)
	{
		m_document->clearCommands();
		m_document->selectNone();
		setCurrentFile(fileName);
		statusBar()->showMessage(tr("File loaded"), 2000);
	}
	else
		statusBar()->showMessage(tr("File imported"), 2000);
	m_graphView->showAll();
	return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
	if (!m_document->writeFile(fileName)) {
		statusBar()->showMessage(tr("Saving failed"), 2000);
		return false;
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File saved"), 2000);
	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	m_curFile = fileName;
	setWindowModified(false);

	QString shownName = tr("Untitled");
	if (!m_curFile.isEmpty()) {
		shownName = strippedName(m_curFile);
		m_recentFiles.removeAll(m_curFile);
		m_recentFiles.prepend(m_curFile);
		updateRecentFileActions();
	}

	setWindowTitle(tr("%1[*] - %2").arg(shownName)
								   .arg(tr("Panda")));
}

void MainWindow::updateRecentFileActions()
{
	QMutableStringListIterator i(m_recentFiles);
	while (i.hasNext()) {
		if (!QFile::exists(i.next()))
			i.remove();
	}

	for (int j = 0; j < MaxRecentFiles; ++j) {
		if (j < m_recentFiles.count()) {
			QString text = tr("&%1 %2")
						   .arg(j + 1)
						   .arg(strippedName(m_recentFiles[j]));
			m_recentFileActions[j]->setText(text);
			m_recentFileActions[j]->setData(m_recentFiles[j]);
			m_recentFileActions[j]->setVisible(true);
		} else {
			m_recentFileActions[j]->setVisible(false);
		}
	}
	m_separatorAction->setVisible(!m_recentFiles.isEmpty());
}

QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::createObject()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if(action)
	{
		auto object = panda::ObjectFactory::getInstance()->create(action->data().toString(), m_document);
		m_document->addCommand(new AddObjectCommand(m_document, m_graphView, object));
	}
}

void MainWindow::switchToGraphView()
{
	m_tabWidget->setCurrentWidget(m_graphView);
}

void MainWindow::switchToOpenGLView()
{
	m_tabWidget->setCurrentWidget(m_openGLRenderView);
}

void MainWindow::switchFullScreen()
{
	m_fullScreen = !m_fullScreen;
	m_fullScreenAction->setChecked(m_fullScreen);

	if(m_fullScreen)
		showFullScreen();
	else
		showNormal();

	bool show = !m_fullScreen;

	menuBar()->setVisible(show);
	statusBar()->setVisible(show);

	m_datasDock->setVisible(show);
	m_layersDock->setVisible(show);

	m_fileToolBar->setVisible(show);
	m_editToolBar->setVisible(show);
	m_animToolBar->setVisible(show);
	m_tabWidget->tabBar()->setVisible(show);
	m_tabWidget->setDocumentMode(m_fullScreen);
}

void MainWindow::adjustRenderSizeToView()
{
	m_adjustRenderSizeToView = !m_adjustRenderSizeToView;
	m_openGLRenderView->setAdjustRenderSize(m_adjustRenderSizeToView);
}

void MainWindow::showStatusBarMessage(QString text)
{
	statusBar()->showMessage(text, 2000);
}

void MainWindow::cut()
{
	m_document->copy();
	del();
}

void MainWindow::paste()
{
	m_document->paste();
	m_graphView->moveSelectedToCenter();
	m_graphView->updateLinkTags();

	auto selection = m_document->getSelection();
	if(!selection.empty())
		m_document->addCommand(new AddObjectCommand(m_document, m_graphView, selection));
}

void MainWindow::del()
{
	auto selection = m_document->getSelection();
	if(!selection.empty())
	{
		auto macro = m_document->beginCommandMacro(tr("delete objects"));
		m_document->addCommand(new RemoveObjectCommand(m_document, m_graphView, selection));
	}
}

void MainWindow::group()
{
	bool res = panda::createGroup(m_document, m_graphView);
	if(!res)
		statusBar()->showMessage(tr("Could not create a group from the selection"), 2000);
}

void MainWindow::ungroup()
{
	bool res = panda::ungroupSelection(m_document, m_graphView);
	if(!res)
		statusBar()->showMessage(tr("Could not ungroup the selection"), 2000);
}

void MainWindow::editGroup()
{
	panda::Group* group = dynamic_cast<panda::Group*>(m_document->getCurrentSelectedObject());
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
	panda::PandaObject* object = m_document->getCurrentSelectedObject();
	panda::Group* group = dynamic_cast<panda::Group*>(object);
	if(group)
	{
		if(GroupsManager::getInstance()->saveGroup(group))
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
		GroupsManager::getInstance()->createGroupObject(m_document, m_graphView, path);
	}
}

void MainWindow::showContextMenu(QPoint pos, int flags)
{
	QMenu menu(this);

	panda::PandaObject* obj = m_document->getCurrentSelectedObject();
	if(obj)
	{
		menu.addAction(m_cutAction);
		menu.addAction(m_copyAction);
	}
	menu.addAction(m_pasteAction);

	if(flags & GraphView::MENU_LINK)
	{
		menu.addAction(m_removeLinkAction);
	}

	const panda::BaseData* clickedData = m_graphView->getContextMenuData();
	if(clickedData && clickedData->isDisplayed())
		menu.addAction(m_copyDataAction);

	int nbSelected = m_document->getSelection().size();
	if(dynamic_cast<panda::Group*>(obj) && nbSelected == 1)
	{
		menu.addAction(m_ungroupAction);
		menu.addAction(m_editGroupAction);
		menu.addAction(m_saveGroupAction);
	}

	if(nbSelected > 1)
	{
		menu.addAction(m_groupAction);
	}

	if(!menu.actions().empty())
		menu.exec(pos);
}

void MainWindow::copyDataToUserValue()
{
	const panda::BaseData* clickedData = m_graphView->getContextMenuData();
	if(clickedData)
		m_document->copyDataToUserValue(clickedData);
}

void MainWindow::showLoggerDialog()
{
	if(!m_loggerDialog)
	{
		m_loggerDialog = new UpdateLoggerDialog(this);
		UpdateLoggerDialog::setInstance(m_loggerDialog);

		connect(m_loggerDialog, SIGNAL(changedSelectedEvent()), m_graphView, SLOT(update()));
	}

	if(m_loggerDialog->isVisible())
		m_loggerDialog->hide();
	else
	{
		m_loggerDialog->updateEvents();
		m_loggerDialog->show();
	}
}

void MainWindow::showObjectsAndTypes()
{
	QString fileName = "file:///" + createObjectsAndTypesPage(m_document);
	QDesktopServices::openUrl(QUrl(fileName));
}

void MainWindow::play(bool playing)
{
	m_cutAction->setEnabled(!playing);
	m_pasteAction->setEnabled(!playing);
	m_deleteAction->setEnabled(!playing);
	m_removeLinkAction->setEnabled(!playing);
	m_copyDataAction->setEnabled(!playing);

	m_registryMenu->setEnabled(!playing);

	m_importAction->setEnabled(!playing);

	if(playing)
	{
		m_undoEnabled = m_undoAction->isEnabled();
		m_redoEnabled = m_redoAction->isEnabled();
		m_undoAction->setEnabled(false);
		m_redoAction->setEnabled(false);
	}
	else
	{
		m_undoAction->setEnabled(m_undoEnabled);
		m_redoAction->setEnabled(m_redoEnabled);
	}
}

void MainWindow::selectedObject(panda::PandaObject* object)
{
	int nbSelected = m_document->getNbSelected();
	bool isGroup = (nbSelected == 1) && dynamic_cast<panda::Group*>(object);

	m_ungroupAction->setEnabled(isGroup);
	m_editGroupAction->setEnabled(isGroup);
	m_saveGroupAction->setEnabled(isGroup);

	m_groupAction->setEnabled(nbSelected > 1);
}
