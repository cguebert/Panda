#include <QtWidgets>

#include <ui/CreateGroup.h>
#include <ui/DatasTable.h>
#include <ui/DetachableTabWidget.h>
#include <ui/EditGroupDialog.h>
#include <ui/GraphView.h>
#include <ui/GroupsManager.h>
#include <ui/ImageViewport.h>
#include <ui/OpenGLRenderView.h>
#include <ui/LayersTab.h>
#include <ui/ListObjectsAndTypes.h>
#include <ui/MainWindow.h>
#include <ui/ScrollContainer.h>
#include <ui/SimpleGUIImpl.h>
#include <ui/UpdateLoggerDialog.h>

#include <ui/command/AddObjectCommand.h>
#include <ui/command/RemoveObjectCommand.h>

#include <ui/graphview/alignObjects.h>
#include <ui/graphview/ObjectsSelection.h>

#include <panda/PandaDocument.h>
#include <panda/types/DataTraits.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/Serialization.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Group.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/PluginsManager.h>

#include <iostream>

MainWindow::MainWindow()
{
	// This is not working with Intel GPUs, needs more testing
/*	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 0);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);*/
	
	m_simpleGUI = new SimpleGUIImpl(this);
	m_document = std::make_unique<panda::PandaDocument>(*m_simpleGUI);

	m_graphView = new GraphView(m_document.get());
	m_graphViewContainer = new ScrollContainer();
	m_graphViewContainer->setFrameStyle(0); // No frame
	m_graphViewContainer->setView(m_graphView);

	m_openGLRenderView = new OpenGLRenderView(m_document.get());
	m_openGLViewContainer = new QScrollArea();
	m_openGLViewContainer->setFrameStyle(0);
	m_openGLViewContainer->setAlignment(Qt::AlignCenter);
	m_openGLViewContainer->setWidget(m_openGLRenderView);
	m_openGLViewContainer->setWidgetResizable(true);

	m_tabWidget = new DetachableTabWidget;
	m_tabWidget->addTab(m_graphViewContainer, tr("Graph"));
	m_tabWidget->addTab(m_openGLViewContainer, tr("Render"));
	m_tabWidget->setCurrentWidget(m_openGLViewContainer); // First go to the OpenGL view (initialize it)
	setCentralWidget(m_tabWidget);

	// Go back to the graph view asap
	QTimer::singleShot(0, [this](){
		m_tabWidget->setCurrentWidget(m_graphViewContainer);
		m_graphView->setFocus();
	});

	// Set the application directories
	auto& dataRepository = panda::helper::system::DataRepository;
	auto standardPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	for(const auto& path : standardPaths)
		dataRepository.addPath(path.toStdString());
	dataRepository.addPath(QCoreApplication::applicationDirPath().toStdString());
	auto fontPaths = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
	for (const auto& path : fontPaths)
		dataRepository.addPath(path.toStdString());

	panda::PluginsManager::getInstance()->loadPlugins();

	createActions();
	createStatusBar();

	m_observer.get(m_document->getSignals().modified).connect<MainWindow, &MainWindow::documentModified>(this);
	m_observer.get(m_graphView->selection().selectedObject).connect<MainWindow, &MainWindow::selectedObject>(this);
	m_observer.get(m_document->getSignals().removedObject).connect<MainWindow, &MainWindow::removedObject>(this);

	m_observer.get(m_document->getUndoStack().m_canUndoChangedSignal).connect<MainWindow, &MainWindow::undoEnabled>(this);
	m_observer.get(m_document->getUndoStack().m_canRedoChangedSignal).connect<MainWindow, &MainWindow::redoEnabled>(this);
	m_observer.get(m_document->getUndoStack().m_undoTextChangedSignal).connect<MainWindow, &MainWindow::undoTextChanged>(this);
	m_observer.get(m_document->getUndoStack().m_redoTextChangedSignal).connect<MainWindow, &MainWindow::redoTextChanged>(this);

	connect(m_graphView, SIGNAL(modified()), this, SLOT(documentModified()));
	connect(m_graphView, SIGNAL(showStatusBarMessage(QString)), this, SLOT(showStatusBarMessage(QString)));
	connect(m_graphView, SIGNAL(showContextMenu(QPoint,int)), this, SLOT(showContextMenu(QPoint,int)));
	connect(m_tabWidget, SIGNAL(openDetachedWindow(DetachedWindow*)), this, SLOT(openDetachedWindow(DetachedWindow*)));

	connect(m_graphView, &GraphView::lostFocus, this, &MainWindow::onTabWidgetFocusLoss);
	connect(m_openGLRenderView, &OpenGLRenderView::lostFocus, this, &MainWindow::onTabWidgetFocusLoss);

	createGroupRegistryMenu();

	setWindowIcon(QIcon(":/share/icons/icon.png"));
	setCurrentFile("");

	m_datasTable = new DatasTable(m_graphView, this);

	m_datasDock = new QDockWidget(tr("Properties"), this);
	m_datasDock->setObjectName("PropertiesDock");
	m_datasDock->setWidget(m_datasTable);
	m_datasDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_datasDock);

	m_layersTab = new LayersTab(m_document.get(), this);

	m_layersDock = new QDockWidget(tr("Layers"), this);
	m_layersDock->setObjectName("LayersDock");
	m_layersDock->setWidget(m_layersTab);
	m_layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_layersDock);

	readSettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (okToContinue())
	{
		writeSettings();
		event->accept();
	}
	else
		event->ignore();
}

void MainWindow::newFile()
{
	if (okToContinue())
	{
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

			auto selection = m_graphView->selection().get();
			if(!selection.empty())
				m_document->getUndoStack().push(std::make_shared<AddObjectCommand>(m_document.get(), m_graphView, selection));
		}
	}
}

bool MainWindow::save()
{
	if (m_curFile.isEmpty())
		return saveAs();
	else
		return saveFile(m_curFile);
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
			tr("<h2>Panda 0.4</h2>"
			   "<p>Copyright &copy; 2015 Christophe Guébert"
			   "<p>Panda is a framework for procedural drawing and animation."));
}

void MainWindow::openRecentFile()
{
	if (okToContinue())
	{
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
	float time = m_document->getAnimationTime();
	float FPS = m_document->getFPS();
	m_timeLabel->setText(tr("time: %1\tFPS: %2").arg(time).arg(QString::number(FPS, 'f', 1)));
}

void MainWindow::documentModified()
{
	if(m_openGLRenderView->isVisible())
		m_openGLRenderView->update();

	setWindowModified(true);
}

void MainWindow::createActions()
{
/*** Creation of actions ***/
	auto newAction = new QAction(tr("&New"), this);
	newAction->setIcon(QIcon(":/share/icons/new.png"));
	newAction->setShortcut(QKeySequence::New);
	newAction->setStatusTip(tr("Create a new panda document"));
	connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

	auto openAction = new QAction(tr("&Open..."), this);
	openAction->setIcon(QIcon(":/share/icons/open.png"));
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("Open an existing panda document"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_importAction = new QAction(tr("&Import..."), this);
	m_importAction->setIcon(QIcon(":/share/icons/open.png"));
	m_importAction->setStatusTip(tr("Import an existing panda document into the current one"));
	connect(m_importAction, SIGNAL(triggered()), this, SLOT(import()));

	auto saveAction = new QAction(tr("&Save"), this);
	saveAction->setIcon(QIcon(":/share/icons/save.png"));
	saveAction->setShortcut(QKeySequence::Save);
	saveAction->setStatusTip(tr("Save the document to disk"));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

	auto saveAsAction = new QAction(tr("Save &As..."), this);
	saveAsAction->setShortcut(QKeySequence::SaveAs);
	saveAsAction->setStatusTip(tr("Save the document under a new "
								  "name"));
	connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	auto exitAction = new QAction(tr("E&xit"), this);
	exitAction->setShortcut(tr("Ctrl+Q"));
	exitAction->setStatusTip(tr("Exit Panda"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

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
	connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copy()));
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

	auto selectAllAction = new QAction(tr("Select &all"), this);
	selectAllAction->setShortcut(QKeySequence::SelectAll);
	selectAllAction->setStatusTip(tr("Select all objects"));
	connect(selectAllAction, &QAction::triggered, [this]() { m_graphView->selection().selectAll(); });
	addAction(selectAllAction);

	auto selectNoneAction = new QAction(tr("Select &none"), this);
	selectNoneAction->setShortcut(tr("Ctrl+Shift+A"));
	selectNoneAction->setStatusTip(tr("Deselect all objets"));
	connect(selectNoneAction, &QAction::triggered, [this]() { m_graphView->selection().selectNone(); });
	addAction(selectNoneAction);

	auto selectConnectedAction = new QAction(tr("Select &connected"), this);
	selectConnectedAction->setShortcut(tr("Ctrl+Shift+C"));
	selectConnectedAction->setStatusTip(tr("Select all objects connected to the current one"));
	connect(selectConnectedAction, &QAction::triggered, [this]() { m_graphView->selection().selectConnected(); });
	addAction(selectConnectedAction);

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

	auto zoomResetAction = new QAction(tr("Reset &zoom"), this);
	zoomResetAction->setShortcut(tr("Ctrl+0"));
	zoomResetAction->setStatusTip(tr("Set zoom to 100%"));
	zoomResetAction->setShortcutContext(Qt::WidgetShortcut);
	connect(zoomResetAction, SIGNAL(triggered()), m_graphView, SLOT(zoomReset()));
	m_graphView->addAction(zoomResetAction);

	auto zoomInAction = new QAction(tr("Zoom &in"), m_graphView);
	zoomInAction->setShortcut(QKeySequence::ZoomIn);
	zoomInAction->setStatusTip(tr("Zoom in"));
	zoomInAction->setShortcutContext(Qt::WidgetShortcut);
	connect(zoomInAction, SIGNAL(triggered()), m_graphView, SLOT(zoomIn()));
	m_graphView->addAction(zoomInAction);

	auto zoomOutAction = new QAction(tr("Zoom &out"), m_graphView);
	zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	zoomOutAction->setStatusTip(tr("Zoom out"));
	zoomOutAction->setShortcutContext(Qt::WidgetShortcut);
	connect(zoomOutAction, SIGNAL(triggered()), m_graphView, SLOT(zoomOut()));
	m_graphView->addAction(zoomOutAction);

	auto centerViewAction = new QAction(tr("&Center view"), this);
	centerViewAction->setShortcut(tr("Ctrl+5"));
	centerViewAction->setStatusTip(tr("Center the view"));
	centerViewAction->setShortcutContext(Qt::WidgetShortcut);
	connect(centerViewAction, SIGNAL(triggered()), m_graphView, SLOT(centerView()));
	m_graphView->addAction(centerViewAction);

	auto showAllAction = new QAction(tr("Show &all"), this);
	showAllAction->setShortcut(tr("Ctrl+f"));
	showAllAction->setStatusTip(tr("Center and zoom the view so that all objects are visible"));
	showAllAction->setShortcutContext(Qt::WidgetShortcut);
	connect(showAllAction, SIGNAL(triggered()), m_graphView, SLOT(showAll()));
	m_graphView->addAction(showAllAction);

	auto showAllSelectedAction = new QAction(tr("Show all &selected"), this);
	showAllSelectedAction->setShortcut(tr("Ctrl+d"));
	showAllSelectedAction->setStatusTip(tr("Center and zoom the view so that all selected objects are visible"));
	showAllSelectedAction->setShortcutContext(Qt::WidgetShortcut);
	connect(showAllSelectedAction, SIGNAL(triggered()), m_graphView, SLOT(showAllSelected()));
	m_graphView->addAction(showAllSelectedAction);

	auto showGraphViewAction = new QAction(tr("Show &graph view"), this);
	showGraphViewAction->setShortcut(tr("Ctrl+1"));
	showGraphViewAction->setStatusTip(tr("Switch to the graph view"));
	connect(showGraphViewAction, SIGNAL(triggered()), this, SLOT(switchToGraphView()));
	addAction(showGraphViewAction);

	auto showOpenGLViewAction = new QAction(tr("Show &render view"), this);
	showOpenGLViewAction->setShortcut(tr("Ctrl+2"));
	showOpenGLViewAction->setStatusTip(tr("Switch to the render view"));
	connect(showOpenGLViewAction, SIGNAL(triggered()), this, SLOT(switchToOpenGLView()));
	addAction(showOpenGLViewAction);

	auto adjustRenderSizeToViewAction = new QAction(tr("Adjust size to &view"), this);
	adjustRenderSizeToViewAction->setShortcut(tr("F10"));
	adjustRenderSizeToViewAction->setStatusTip(tr("Adjust the render size to the view"));
	adjustRenderSizeToViewAction->setCheckable(true);
	connect(adjustRenderSizeToViewAction, SIGNAL(triggered()), this, SLOT(adjustRenderSizeToView()));
	addAction(adjustRenderSizeToViewAction);

	m_fullScreenAction = new QAction(tr("&Full screen"), this);
	m_fullScreenAction->setShortcut(tr("F11"));
	m_fullScreenAction->setStatusTip(tr("Put the application in full screen mode"));
	m_fullScreenAction->setCheckable(true);
	connect(m_fullScreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreen);
	addAction(m_fullScreenAction);

	auto exitFullscreenOnFocusLossAction = new QAction(tr("Automatically exit fullscreen"), this);
	exitFullscreenOnFocusLossAction->setStatusTip(tr("Exit the fullscreen mode if the window lose the focus"));
	exitFullscreenOnFocusLossAction->setCheckable(true);
	exitFullscreenOnFocusLossAction->setChecked(true);
	connect(exitFullscreenOnFocusLossAction, &QAction::triggered, [this]() { m_exitFullscreenOnFocusLoss = !m_exitFullscreenOnFocusLoss; });

	auto aboutAction = new QAction(tr("&About"), this);
	aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	auto aboutQtAction = new QAction(tr("About &Qt"), this);
	aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	m_playAction = new QAction(tr("Play"), this);
	m_playAction->setIcon(QIcon(":/share/icons/play.png"));
	m_playAction->setShortcut(tr("F5"));
	m_playAction->setStatusTip(tr("Start the animation"));
	m_playAction->setCheckable(true);
	connect(m_playAction, &QAction::triggered, [this](bool playing) { m_document->play(playing); });
	connect(m_playAction, SIGNAL(triggered(bool)), this, SLOT(play(bool)));
	addAction(m_playAction);

	auto stepAction = new QAction(tr("Step"), this);
	stepAction->setIcon(QIcon(":/share/icons/step.png"));
	stepAction->setShortcut(tr("F6"));
	stepAction->setStatusTip(tr("Do one step of the animation"));
	connect(stepAction, &QAction::triggered, [this]() { m_document->step(); });
	addAction(stepAction);

	auto rewindAction = new QAction(tr("Rewind"), this);
	rewindAction->setIcon(QIcon(":/share/icons/stop.png"));
	rewindAction->setShortcut(tr("F7"));
	rewindAction->setStatusTip(tr("Rewind the animation back to the begining"));
	connect(rewindAction, &QAction::triggered, [this]() { m_document->rewind(); });
	addAction(rewindAction);

	m_removeLinkAction = new QAction(tr("Remove link"), this);
	m_removeLinkAction->setStatusTip(tr("Remove the link to this data"));
	connect(m_removeLinkAction, SIGNAL(triggered()), m_graphView, SLOT(removeLink()));
	addAction(m_removeLinkAction);

	m_copyDataAction = new QAction(tr("Copy data"), this);
	m_copyDataAction->setStatusTip(tr("Create a user value generator based on this data"));
	connect(m_copyDataAction, SIGNAL(triggered()), this, SLOT(copyDataToUserValue()));
	addAction(m_copyDataAction);

	auto showLoggerDialogAction = new QAction(tr("Show &log"), this);
	showLoggerDialogAction->setStatusTip(tr("Show the updates log dialog"));
	connect(showLoggerDialogAction, SIGNAL(triggered()), this, SLOT(showLoggerDialog()));

	auto showDirtyInfoAction = new QAction(tr("Debug dirty state"), this);
	showDirtyInfoAction->setCheckable(true);
	showDirtyInfoAction->setStatusTip(tr("Show the dirty status of each object and data"));
	connect(showDirtyInfoAction, &QAction::triggered, m_graphView, &GraphView::debugDirtyState);

	auto showObjectsAndTypesAction = new QAction(tr("List types and objects"), this);
	showObjectsAndTypesAction->setStatusTip(tr("Show information about all available types and objects"));
	connect(showObjectsAndTypesAction, SIGNAL(triggered()), this, SLOT(showObjectsAndTypes()));

	m_showImageViewport = new QAction(tr("Open image viewport"), this);
	m_showImageViewport->setStatusTip(tr("Open a new viewport to an image"));
	connect(m_showImageViewport, SIGNAL(triggered()), this, SLOT(showImageViewport()));
	addAction(m_showImageViewport);

	m_chooseWidget = new QAction(tr("Choose widget"), this);
	m_chooseWidget->setStatusTip(tr("Choose the widget to use for this data"));
	connect(m_chooseWidget, SIGNAL(triggered()), m_graphView, SLOT(showChooseWidgetDialog()));
	addAction(m_chooseWidget);

	m_undoAction = new QAction(tr("Undo"), this);
	m_undoAction->setShortcut(QKeySequence::Undo);
	m_undoAction->setEnabled(false);
	connect(m_undoAction, &QAction::triggered, [this]() { m_document->getUndoStack().undo(); });
	addAction(m_undoAction);

	m_redoAction = new QAction(tr("Redo"), this);
	m_redoAction->setShortcut(QKeySequence::Redo);
	m_redoAction->setEnabled(false);
	connect(m_redoAction, &QAction::triggered, [this]() { m_document->getUndoStack().redo(); });
	addAction(m_redoAction);

	auto convertDocumentsAction = new QAction(tr("Convert documents"), this);
	connect(convertDocumentsAction, SIGNAL(triggered()), this, SLOT(convertSavedDocuments()));

	// Alignment (horizontal)
	auto alignHorCenterAction = new QAction(tr("Center"), this);
	alignHorCenterAction->setIcon(QIcon(":/share/icons/align-horizontal-center.png"));
	alignHorCenterAction->setStatusTip(tr("Center on vertical axis"));
	connect(alignHorCenterAction, &QAction::triggered, [this]() { alignHorizontallyCenter(m_graphView); });
	addAction(alignHorCenterAction);

	auto alignHorLeftAction = new QAction(tr("Left"), this);
	alignHorLeftAction->setIcon(QIcon(":/share/icons/align-horizontal-left.png"));
	alignHorLeftAction->setStatusTip(tr("Align left edges"));
	connect(alignHorLeftAction, &QAction::triggered, [this]() { alignHorizontallyLeft(m_graphView); });
	addAction(alignHorLeftAction);

	auto alignHorRightAction = new QAction(tr("Right"), this);
	alignHorRightAction->setIcon(QIcon(":/share/icons/align-horizontal-right.png"));
	alignHorRightAction->setStatusTip(tr("Align right edges"));
	connect(alignHorRightAction, &QAction::triggered, [this]() { alignHorizontallyRight(m_graphView); });
	addAction(alignHorRightAction);

	// Alignment (vertical)
	auto alignVertCenterAction = new QAction(tr("Center"), this);
	alignVertCenterAction->setIcon(QIcon(":/share/icons/align-vertical-center.png"));
	alignVertCenterAction->setStatusTip(tr("Center on horizontal axis"));
	connect(alignVertCenterAction, &QAction::triggered, [this]() { alignVerticallyCenter(m_graphView); });
	addAction(alignVertCenterAction);

	auto alignVertTopAction = new QAction(tr("Top"), this);
	alignVertTopAction->setIcon(QIcon(":/share/icons/align-vertical-top.png"));
	alignVertTopAction->setStatusTip(tr("Align top edges"));
	connect(alignVertTopAction, &QAction::triggered, [this]() { alignVerticallyTop(m_graphView); });
	addAction(alignVertTopAction);

	auto alignVertBottomAction = new QAction(tr("Bottom"), this);
	alignVertBottomAction->setIcon(QIcon(":/share/icons/align-vertical-bottom.png"));
	alignVertBottomAction->setStatusTip(tr("Align bottom edges"));
	connect(alignVertBottomAction, &QAction::triggered, [this]() { alignVerticallyBottom(m_graphView); });
	addAction(alignVertBottomAction);

	// Distribution (horizontal)
	auto distributeHorCenterAction = new QAction(tr("Center"), this);
	distributeHorCenterAction->setIcon(QIcon(":/share/icons/distribute-horizontal-center.png"));
	distributeHorCenterAction->setStatusTip(tr("Distribute centers equidistantly horizontally"));
	connect(distributeHorCenterAction, &QAction::triggered, [this]() { distributeHorizontallyCenter(m_graphView); });
	addAction(distributeHorCenterAction);

	auto distributeHorGapsAction = new QAction(tr("Gaps"), this);
	distributeHorGapsAction->setIcon(QIcon(":/share/icons/distribute-horizontal-gaps.png"));
	distributeHorGapsAction->setStatusTip(tr("Make horizontal gaps between objects equal"));
	connect(distributeHorGapsAction, &QAction::triggered, [this]() { distributeHorizontallyGaps(m_graphView); });
	addAction(distributeHorGapsAction);

	auto distributeHorLeftAction = new QAction(tr("Left"), this);
	distributeHorLeftAction->setIcon(QIcon(":/share/icons/distribute-horizontal-left.png"));
	distributeHorLeftAction->setStatusTip(tr("Distribute left edges equidistantly"));
	connect(distributeHorLeftAction, &QAction::triggered, [this]() { distributeHorizontallyLeft(m_graphView); });
	addAction(distributeHorLeftAction);

	auto distributeHorRightAction = new QAction(tr("Right"), this);
	distributeHorRightAction->setIcon(QIcon(":/share/icons/distribute-horizontal-right.png"));
	distributeHorRightAction->setStatusTip(tr("Distribute left edges equidistantly"));
	connect(distributeHorRightAction, &QAction::triggered, [this]() { distributeHorizontallyRight(m_graphView); });
	addAction(distributeHorRightAction);

	// Distribution (vertical)
	auto distributeVertCenterAction = new QAction(tr("Center"), this);
	distributeVertCenterAction->setIcon(QIcon(":/share/icons/distribute-vertical-center.png"));
	distributeVertCenterAction->setStatusTip(tr("Distribute centers equidistantly vertically"));
	connect(distributeVertCenterAction, &QAction::triggered, [this]() { distributeVerticallyCenter(m_graphView); });
	addAction(distributeVertCenterAction);

	auto distributeVertGapsAction = new QAction(tr("Gaps"), this);
	distributeVertGapsAction->setIcon(QIcon(":/share/icons/distribute-vertical-gaps.png"));
	distributeVertGapsAction->setStatusTip(tr("Make vertical gaps between objects equal"));
	connect(distributeVertGapsAction, &QAction::triggered, [this]() { distributeVerticallyGaps(m_graphView); });
	addAction(distributeVertGapsAction);

	auto distributeVertTopAction = new QAction(tr("Top"), this);
	distributeVertTopAction->setIcon(QIcon(":/share/icons/distribute-vertical-top.png"));
	distributeVertTopAction->setStatusTip(tr("Distribute top edges equidistantly"));
	connect(distributeVertTopAction, &QAction::triggered, [this]() { distributeVerticallyTop(m_graphView); });
	addAction(distributeVertTopAction);

	auto distributeVertBottomAction = new QAction(tr("Bottom"), this);
	distributeVertBottomAction->setIcon(QIcon(":/share/icons/distribute-vertical-bottom.png"));
	distributeVertBottomAction->setStatusTip(tr("Distribute bottom edges equidistantly"));
	connect(distributeVertBottomAction, &QAction::triggered, [this]() { distributeVerticallyBottom(m_graphView); });
	addAction(distributeVertBottomAction);

/*** Creation of menus ***/
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(newAction);
	m_fileMenu->addAction(openAction);
	m_fileMenu->addAction(m_importAction);
	m_fileMenu->addAction(saveAction);
	m_fileMenu->addAction(saveAsAction);
	m_separatorAction = m_fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		m_fileMenu->addAction(m_recentFileActions[i]);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(exitAction);

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
	m_selectMenu->addAction(selectAllAction);
	m_selectMenu->addAction(selectNoneAction);
	m_selectMenu->addAction(selectConnectedAction);

	m_groupMenu = m_editMenu->addMenu(tr("&Group"));
	m_groupMenu->addAction(m_groupAction);
	m_groupMenu->addAction(m_ungroupAction);
	m_groupMenu->addAction(m_editGroupAction);
	m_groupMenu->addAction(m_saveGroupAction);

	m_editMenu->addSeparator();
	m_alignHorizontallyMenu = m_editMenu->addMenu(tr("Align horizontally"));
	m_alignHorizontallyMenu->addAction(alignHorLeftAction);
	m_alignHorizontallyMenu->addAction(alignHorCenterAction);
	m_alignHorizontallyMenu->addAction(alignHorRightAction);

	m_alignVerticallyMenu = m_editMenu->addMenu(tr("Align vertically"));
	m_alignVerticallyMenu->addAction(alignVertTopAction);
	m_alignVerticallyMenu->addAction(alignVertCenterAction);
	m_alignVerticallyMenu->addAction(alignVertBottomAction);

	m_distributeHorizontallyMenu = m_editMenu->addMenu(tr("Distribute horizontally"));
	m_distributeHorizontallyMenu->addAction(distributeHorLeftAction);
	m_distributeHorizontallyMenu->addAction(distributeHorCenterAction);
	m_distributeHorizontallyMenu->addAction(distributeHorRightAction);
	m_distributeHorizontallyMenu->addAction(distributeHorGapsAction);

	m_distributeVerticallyMenu = m_editMenu->addMenu(tr("Distribute vertically"));
	m_distributeVerticallyMenu->addAction(distributeVertTopAction);
	m_distributeVerticallyMenu->addAction(distributeVertCenterAction);
	m_distributeVerticallyMenu->addAction(distributeVertBottomAction);
	m_distributeVerticallyMenu->addAction(distributeVertGapsAction);

	createRegistryMenu();

	m_viewMenu = menuBar()->addMenu(tr("&View"));
	m_viewMenu->addAction(zoomInAction);
	m_viewMenu->addAction(zoomOutAction);
	m_viewMenu->addAction(zoomResetAction);
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(centerViewAction);
	m_viewMenu->addAction(showAllAction);
	m_viewMenu->addAction(showAllSelectedAction);
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(showGraphViewAction);
	m_viewMenu->addAction(showOpenGLViewAction);
	m_viewMenu->addAction(adjustRenderSizeToViewAction);
	m_viewMenu->addAction(m_fullScreenAction);
	m_viewMenu->addAction(exitFullscreenOnFocusLossAction);
#ifdef PANDA_LOG_EVENTS
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(showLoggerDialogAction);
	m_viewMenu->addAction(showDirtyInfoAction);
#endif

	menuBar()->addSeparator();

	m_helpMenu = menuBar()->addMenu(tr("&Help"));
	m_helpMenu->addAction(showObjectsAndTypesAction);
//	m_helpMenu->addAction(convertDocumentsAction);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(aboutAction);
	m_helpMenu->addAction(aboutQtAction);

/*** Creation of toolbars ***/
	m_fileToolBar = addToolBar(tr("&File"));
	m_fileToolBar->setObjectName("FileToolBar");
	m_fileToolBar->addAction(newAction);
	m_fileToolBar->addAction(openAction);
	m_fileToolBar->addAction(saveAction);

	m_editToolBar = addToolBar(tr("&Edit"));
	m_editToolBar->setObjectName("EditToolBar");
	m_editToolBar->addAction(m_cutAction);
	m_editToolBar->addAction(m_copyAction);
	m_editToolBar->addAction(m_pasteAction);

	m_animToolBar = addToolBar(tr("&Animation"));
	m_animToolBar->setObjectName("AnimToolBar");
	m_animToolBar->addAction(m_playAction);
	m_animToolBar->addAction(stepAction);
	m_animToolBar->addAction(rewindAction);
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

			QString display = QString::fromStdString(entry.menuDisplay);
			QStringList hierarchy = display.split("/");
			menuItemInfo* currentMenu = &menuTree;
			for(int i=0; i<hierarchy.count()-1; ++i)
				currentMenu = &currentMenu->childs[hierarchy[i]];

			QAction* tempAction = new QAction(hierarchy.last(), this);
			if(!entry.description.empty())
				tempAction->setStatusTip(QString::fromStdString(entry.description));
			tempAction->setData(QString::fromStdString(entry.className));
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

void MainWindow::createStatusBar()
{
	m_timeLabel = new QLabel(tr("time: %1\t FPS: %2").arg(999.99).arg(999.9));
	m_timeLabel->setAlignment(Qt::AlignLeft);
	m_timeLabel->setMinimumSize(m_timeLabel->sizeHint());

	statusBar()->addWidget(m_timeLabel);

	m_observer.get(m_document->getSignals().timeChanged).connect<MainWindow, &MainWindow::updateStatusBar>(this);

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
	if (isWindowModified())
	{
		int r = QMessageBox::warning(this, tr("Panda"),
						tr("The document has been modified.\n"
						   "Do you want to save your changes?"),
						QMessageBox::Yes | QMessageBox::No
						| QMessageBox::Cancel);
		if (r == QMessageBox::Yes)
			return save();
		else if (r == QMessageBox::Cancel)
			return false;
	}
	return true;
}

bool MainWindow::loadFile(const QString &fileName, bool import)
{
	auto result = panda::serialization::readFile(m_document.get(), fileName.toStdString(), import);

	if(!result.first)
	{
		statusBar()->showMessage(tr("Loading failed"), 2000);
		return false;
	}

	m_graphView->selection().set(result.second);

	if(!import)
	{
		m_document->getUndoStack().clear();
		m_graphView->selection().selectNone();
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
	if (!panda::serialization::writeFile(m_document.get(), fileName.toStdString()))
	{
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
	if (!m_curFile.isEmpty())
	{
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

	for (int j = 0; j < MaxRecentFiles; ++j)
	{
		if (j < m_recentFiles.count())
		{
			QString text = tr("&%1 %2")
						   .arg(j + 1)
						   .arg(strippedName(m_recentFiles[j]));
			m_recentFileActions[j]->setText(text);
			m_recentFileActions[j]->setData(m_recentFiles[j]);
			m_recentFileActions[j]->setVisible(true);
		}
		else
			m_recentFileActions[j]->setVisible(false);
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
		auto object = panda::ObjectFactory::getInstance()->create(action->data().toString().toStdString(), m_document.get());
		m_document->getUndoStack().push(std::make_shared<AddObjectCommand>(m_document.get(), m_graphView, object));
	}
}

void MainWindow::switchToGraphView()
{
	m_tabWidget->setCurrentWidget(m_graphViewContainer);
}

void MainWindow::switchToOpenGLView()
{
	m_tabWidget->setCurrentWidget(m_openGLViewContainer);
}

void MainWindow::toggleFullScreen(bool fullscreen)
{
	if (m_fullScreen == fullscreen)
		return;

	m_fullScreen = fullscreen;
	m_fullScreenAction->setChecked(m_fullScreen);

	if (m_fullScreen)
	{
		showFullScreen();
		auto w = selectedTabWidget();
		if (w)
			w->setFocus();
	}
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
	m_openGLViewContainer->setWidgetResizable(m_adjustRenderSizeToView);
}

void MainWindow::showStatusBarMessage(QString text)
{
	statusBar()->showMessage(text, 2000);
}

void MainWindow::copy()
{
	if (m_graphView->selection().get().empty())
		return;

	auto objects = m_graphView->selection().get();
	QApplication::clipboard()->setText(QString::fromStdString(panda::serialization::writeTextDocument(m_document.get(), objects)));
}

void MainWindow::cut()
{
	copy();
	del();
}

void MainWindow::paste()
{
	const QMimeData* mimeData = QApplication::clipboard()->mimeData();
	if (!mimeData->hasText())
		return;
	
	auto result = panda::serialization::readTextDocument(m_document.get(), mimeData->text().toStdString());
	if (!result.first || result.second.empty())
		return;

	m_graphView->selection().set(result.second);

	m_graphView->moveSelectedToCenter();
	m_graphView->updateLinkTags();

	m_document->getUndoStack().push(std::make_shared<AddObjectCommand>(m_document.get(), m_graphView, result.second));
}

void MainWindow::del()
{
	auto selection = m_graphView->selection().get();
	if(!selection.empty())
	{
		auto macro = m_document->getUndoStack().beginMacro(tr("delete objects").toStdString());
		m_document->getUndoStack().push(std::make_shared<RemoveObjectCommand>(m_document.get(), m_graphView, selection));
	}
}

void MainWindow::group()
{
	bool res = panda::createGroup(m_document.get(), m_graphView);
	if(!res)
		statusBar()->showMessage(tr("Could not create a group from the selection"), 2000);
}

void MainWindow::ungroup()
{
	bool res = panda::ungroupSelection(m_document.get(), m_graphView);
	if(!res)
		statusBar()->showMessage(tr("Could not ungroup the selection"), 2000);
}

void MainWindow::editGroup()
{
	panda::Group* group = dynamic_cast<panda::Group*>(m_graphView->selection().lastSelectedObject());
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
	panda::PandaObject* object = m_graphView->selection().lastSelectedObject();
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
	QAction *action = qobject_cast<QAction*>(sender());
	if(action)
	{
		QString path = action->data().toString();
		GroupsManager::getInstance()->createGroupObject(m_document.get(), m_graphView, path);
	}
}

void MainWindow::showContextMenu(QPoint pos, int flags)
{
	QMenu menu(this);

	panda::PandaObject* obj = m_graphView->selection().lastSelectedObject();
	if(obj)
	{
		menu.addAction(m_cutAction);
		menu.addAction(m_copyAction);
	}
	menu.addAction(m_pasteAction);

	if(flags & GraphView::MENU_LINK)
		menu.addAction(m_removeLinkAction);

	if(flags & GraphView::MENU_DATA)
	{
		menu.addAction(m_copyDataAction);
		const panda::PandaObject* owner = nullptr;
		auto data = m_graphView->getContextMenuData();
		if(data)
			owner = data->getOwner();
		if(owner && owner->getClass()->getClassName() == "GeneratorUser" && owner->getClass()->getNamespaceName() == "panda")
			menu.addAction(m_chooseWidget);
	}

	if(obj && obj->getClass()->getClassName() == "GeneratorUser" && obj->getClass()->getNamespaceName() == "panda")
		menu.addAction(m_chooseWidget);

	if(flags & GraphView::MENU_IMAGE)
		menu.addAction(m_showImageViewport);

	int nbSelected = m_graphView->selection().get().size();
	if(nbSelected == 1 && dynamic_cast<panda::Group*>(obj))
	{
		menu.addAction(m_ungroupAction);
		menu.addAction(m_editGroupAction);
		menu.addAction(m_saveGroupAction);
	}

	if(nbSelected > 1)
	{
		menu.addAction(m_groupAction);
		menu.addSeparator();
		menu.addMenu(m_alignHorizontallyMenu);
		menu.addMenu(m_alignVerticallyMenu);
	}

	if (nbSelected > 2)
	{
		menu.addMenu(m_distributeHorizontallyMenu);
		menu.addMenu(m_distributeVerticallyMenu);
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
	QString fileName = "file:///" + createObjectsAndTypesPage(m_document.get());
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
	int nbSelected = m_graphView->selection().get().size();
	bool isGroup = (nbSelected == 1) && dynamic_cast<panda::Group*>(object);

	m_ungroupAction->setEnabled(isGroup);
	m_editGroupAction->setEnabled(isGroup);
	m_saveGroupAction->setEnabled(isGroup);

	m_groupAction->setEnabled(nbSelected > 1);
}

void MainWindow::showImageViewport()
{
	const panda::BaseData* clickedData = m_graphView->getContextMenuData();
	if (!clickedData)
		return;

	auto trait = clickedData->getDataTrait();

	if (trait->isSingleValue())
	{
		auto it = std::find_if(m_imageViewports.begin(), m_imageViewports.end(), [clickedData](const ImageViewportInfo& info) {
			return info.data == clickedData;
		});
		if (it != m_imageViewports.end()) // Do not open another viewport
			return;
	}

	QString label = QString::fromStdString(clickedData->getOwner()->getName()) + "." + QString::fromStdString(clickedData->getName());

	int index = 0;
	if (trait->isVector())
	{
		int nb = trait->size(clickedData->getVoidValue());
		bool ok = true;
		index = QInputDialog::getInt(this, "Image index", "Index of the image to show", 1, 1, nb, 1, &ok); // One based for the GUI
		if (!ok)
			return;

		label += QString(" [%1/%2]").arg(index).arg(nb);
		--index; // Zero based for the viewport
	}

	ImageViewport* imageViewport = new ImageViewport(clickedData, index, this);
	connect(imageViewport, SIGNAL(closeViewport(ImageViewport*)), this, SLOT(closeViewport(ImageViewport*)));
	connect(imageViewport, SIGNAL(destroyedViewport(ImageViewport*)), this, SLOT(destroyedViewport(ImageViewport*)));
	connect(imageViewport, &ImageViewport::lostFocus, this, &MainWindow::onTabWidgetFocusLoss);
	QScrollArea* container = new QScrollArea();
	container->setFrameStyle(0);
	container->setAlignment(Qt::AlignCenter);
	container->setWidget(imageViewport);
	container->setFocusProxy(imageViewport);

	ImageViewportInfo info;
	info.viewport = imageViewport;
	info.container = container;
	info.data = clickedData;
	info.object = clickedData->getOwner();
	m_imageViewports.push_back(info);

	m_tabWidget->addTab(container, label, imageViewport->getDetachableWidgetInfo());
}

void MainWindow::openDetachedWindow(DetachedWindow* window)
{
	m_detachedWindows.push_back(window);
	connect(window, SIGNAL(closeDetachedWindow(DetachedWindow*)), this, SLOT(closeDetachedWindow(DetachedWindow*)));
}

void MainWindow::closeDetachedWindow(DetachedWindow* window)
{
	m_detachedWindows.removeAll(window);
}

void MainWindow::closeViewport(ImageViewport* viewport)
{
	auto it = std::find_if(m_imageViewports.begin(), m_imageViewports.end(), [viewport](const ImageViewportInfo& info) {
		return info.viewport == viewport;
	});
	if (it == m_imageViewports.end())
		return;

	QWidget* container = it->container;
	m_imageViewports.erase(it);
	for(DetachedWindow* window : m_detachedWindows)
	{
		if(window->getTabInfo().widget == container)
		{
			window->closeTab();
			return;
		}
	}

	for(int i = 0, nb = m_tabWidget->count(); i < nb; ++i)
	{
		if(m_tabWidget->widget(i) == container)
		{
			m_tabWidget->closeTab(i);
			return;
		}
	}
}

void MainWindow::destroyedViewport(ImageViewport* viewport)
{
	auto it = std::find_if(m_imageViewports.begin(), m_imageViewports.end(), [viewport](const ImageViewportInfo& info) {
		return info.viewport == viewport;
	});
	if (it != m_imageViewports.end())
		m_imageViewports.erase(it);
}

void MainWindow::removedObject(panda::PandaObject* object)
{
	auto it = std::find_if(m_imageViewports.begin(), m_imageViewports.end(), [object](const ImageViewportInfo& info) {
		return info.object == object;
	});

	if (it != m_imageViewports.end())
		closeViewport(it->viewport);
}

void MainWindow::convertSavedDocuments()
{	// When the save format changes, this can be usd to open and resave all documents in a directory
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("Directory containing documents to convert"));
	QDir dir(dirPath);
	QStringList filters;
	filters << "*.pnd";
	auto entries = dir.entryInfoList(filters, QDir::Files);

	int nb = 0;
	for (const auto& entry : entries)
	{
		auto path = entry.absoluteFilePath();
		m_document->resetDocument();
		if (panda::serialization::readFile(m_document.get(), path.toStdString()).first)
		{
			panda::serialization::writeFile(m_document.get(), path.toStdString());
			++nb;
		}
		else
		{
			if (QMessageBox::Abort == QMessageBox::question(this, 
				tr("Error loading document"), 
				tr("Could not open %1").arg(entry.fileName()), 
				QMessageBox::Abort | QMessageBox::Ignore))
				break;
		}
	}
	m_document->resetDocument();
	setWindowModified(false);

	QMessageBox::information(this, tr("Operation finished"), tr("Converted %1/%2 documents").arg(nb).arg(entries.size()));
}

void MainWindow::undoEnabled(bool enabled)
{
	m_undoAction->setEnabled(enabled);
}

void MainWindow::redoEnabled(bool enabled)
{
	m_redoAction->setEnabled(enabled);
}

void MainWindow::undoTextChanged(const std::string& text)
{
	if (text.empty())
		m_undoAction->setText(tr("Undo"));
	else
		m_undoAction->setText(tr("Undo %1").arg(QString::fromStdString(text)));
}

void MainWindow::redoTextChanged(const std::string& text)
{
	if (text.empty())
		m_redoAction->setText(tr("Redo"));
	else
		m_redoAction->setText(tr("Redo %1").arg(QString::fromStdString(text)));
}

void MainWindow::onTabWidgetFocusLoss(QWidget* w)
{
	if (m_fullScreen && m_exitFullscreenOnFocusLoss && w == selectedTabWidget()) 
		toggleFullScreen(false);
}

namespace
{
	bool hasParent(const QWidget* w, const QWidget* parent)
	{
		auto p = w->parentWidget();
		if (!p)
			return false;
		if (p == parent)
			return true;
		return hasParent(p, parent);
	}
}

QWidget* MainWindow::selectedTabWidget() const
{
	auto currentTabWidget = m_tabWidget->currentWidget();
	if (hasParent(m_openGLRenderView, currentTabWidget))
		return m_openGLRenderView;

	if (hasParent(m_graphView, currentTabWidget))
		return m_graphView;

	for (const auto& imageViewport : m_imageViewports)
	{
		if (hasParent(imageViewport.viewport, currentTabWidget))
			return imageViewport.viewport;
	}

	return nullptr;
}
