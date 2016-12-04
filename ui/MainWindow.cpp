#include <QtWidgets>

#include <ui/DatasTable.h>
#include <ui/ImageViewport.h>
#include <ui/OpenGLRenderView.h>
#include <ui/LayersTab.h>
#include <ui/ListObjectsAndTypes.h>
#include <ui/MainWindow.h>
#include <ui/SimpleGUIImpl.h>

#include <ui/custom/DetachableTabWidget.h>
#include <ui/custom/ScrollContainer.h>

#include <ui/dialog/EditGroupDialog.h>
#include <ui/dialog/UpdateLoggerDialog.h>

#include <ui/graphview/QtViewWrapper.h>

#include <panda/graphview/DocumentView.h>
#include <panda/graphview/alignObjects.h>
#include <panda/graphview/ObjectsSelection.h>
#include <panda/graphview/ViewGUI.h>
#include <panda/graphview/Viewport.h>
#include <panda/graphview/ViewInteraction.h>
#include <panda/graphview/documentdatas/DocumentDatasView.h>

#include <panda/CreateGroup.h>
#include <panda/GroupsManager.h>
#include <panda/PluginsManager.h>
#include <panda/TimedFunctions.h>
#include <panda/VisualizersManager.h>
#include <panda/command/AddObjectCommand.h>
#include <panda/command/RemoveObjectCommand.h>
#include <panda/data/DataFactory.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/InteractiveDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/DocumentUtils.h>
#include <panda/document/ObjectsList.h>
#include <panda/document/Serialization.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/object/Annotation.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Group.h>
#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/types/DataTraits.h>

#include <functional>

namespace
{
	inline panda::types::Point convert(const QPointF& pt)
	{ return panda::types::Point(static_cast<float>(pt.x()), static_cast<float>(pt.y())); }

	inline panda::types::Rect convert(const QRect& r)
	{ return panda::types::Rect(r.left(), r.top(), r.right(), r.bottom()); }
}

// A small class to call a function when a DataNode changes
class DataWatcher : public panda::DataNode
{
public:
	using Callback = std::function<void()>;

	DataWatcher(DataNode& node, Callback func)
	: m_callback(func) 
	{ node.addOutput(*this); }

	void update() override { cleanDirty(); }
	void setDirtyValue(const DataNode*) override { m_callback(); }

private:
	std::function<void()> m_callback;
};

MainWindow::MainWindow()
{
	// This is not working with Intel GPUs, needs more testing
/*	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 0);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);*/

	m_documentViewContainer = new ScrollContainer();
	m_documentViewContainer->setFrameStyle(0); // No frame

	m_openGLViewContainer = new QScrollArea();
	m_openGLViewContainer->setFrameStyle(0);
	m_openGLViewContainer->setAlignment(Qt::AlignCenter);
	m_openGLViewContainer->setWidgetResizable(true);

	// Set the application directories
	auto& dataRepository = panda::helper::system::DataRepository;
	auto standardPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	for(const auto& path : standardPaths)
		dataRepository.addPath(path.toStdString());
	dataRepository.addPath(QCoreApplication::applicationDirPath().toStdString());
	auto fontPaths = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
	for (const auto& path : fontPaths)
		dataRepository.addPath(path.toStdString());

	panda::PluginsManager::loadPlugins();

	createActions();
	createStatusBar();

	createGroupRegistryMenu();

	panda::VisualizersManager::createList();

	setWindowIcon(QIcon(":/share/icons/icon.png"));
	setCurrentFile("");

	m_datasTable = new DatasTable(this);

	m_datasDock = new QDockWidget(tr("Properties"), this);
	m_datasDock->setObjectName("PropertiesDock");
	m_datasDock->setWidget(m_datasTable);
	m_datasDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_datasDock);

	m_layersTab = new LayersTab(this);

	m_layersDock = new QDockWidget(tr("Layers"), this);
	m_layersDock->setObjectName("LayersDock");
	m_layersDock->setWidget(m_layersTab);
	m_layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_layersDock);

	m_simpleGUI = new SimpleGUIImpl(this);

	m_tabWidget = new DetachableTabWidget;
	m_tabWidget->addTab(m_documentViewContainer, tr("Graph"));

	connect(m_tabWidget, &DetachableTabWidget::openDetachedWindow, this, &MainWindow::openDetachedWindow);
	connect(m_tabWidget, &DetachableTabWidget::closedTab, this, &MainWindow::onTabWidgetCloseTab);
	connect(m_tabWidget, &DetachableTabWidget::currentChanged, this, &MainWindow::onTabChanged);

	setDocument(std::make_shared<panda::InteractiveDocument>(*m_simpleGUI));
	
	setCentralWidget(m_tabWidget);

	readSettings();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (okToContinue())
	{
		panda::TimedFunctions::shutdown();
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

		QStringList items;
		items << tr("Basic") << tr("Rendered") << tr("Interactive") << tr("Visualizer");

		bool ok;
		auto item = QInputDialog::getItem(this, tr("New document"), tr("Type of the document:"), items, 2, false, &ok);
		if (ok && !item.isEmpty())
		{
			auto index = items.indexOf(item);
			switch (index)
			{
			case 0: setDocument(std::make_shared<panda::PandaDocument>      (*m_simpleGUI)); break;
			case 1: setDocument(std::make_shared<panda::RenderedDocument>   (*m_simpleGUI)); break;
			case 2: setDocument(std::make_shared<panda::InteractiveDocument>(*m_simpleGUI)); break;
			case 3: 
			{
				items.clear();
				std::map<QString, int> typesMap;
				for (const auto entry : panda::DataFactory::entries())
				{
					auto description = QString::fromStdString(panda::types::DataTraitsList::getTrait(entry->fullType)->typeDescription());
					auto name = QString::fromStdString(entry->typeName);
					auto label = QString("%1 (%2)").arg(description).arg(name);
					items.push_back(label);
					typesMap[label] = entry->fullType;
				}
				items.sort();
				item = QInputDialog::getItem(this, tr("Visualizer data type"), tr("Type of the data to visualize:"), items, 0, false, &ok);
				if (ok && !item.isEmpty())
				{
					auto it = typesMap.find(item);
					if(it != typesMap.end())
					{
						auto doc = std::make_shared<panda::VisualizerDocument>(*m_simpleGUI);
						doc->setVisualizerType(it->second);
						setDocument(doc);
					}
				}
				break;
			}
			}
		}

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
			loadFile(fileName);
	}
}

void MainWindow::import()
{
	QString fileName = QFileDialog::getOpenFileName(this,
							   tr("Open Document"), ".",
							   tr("Panda files (*.pnd);;XML Files (*.xml)"));
	if (!fileName.isEmpty())
	{
		if(importFile(fileName))
		{
			auto selection = m_documentView->view().selection().get();
			if(!selection.empty())
				m_document->getUndoStack().push(std::make_shared<panda::AddObjectCommand>(m_document.get(), m_document->getObjectsList(), selection));
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
			tr("<h2>Panda 0.5</h2>"
			   "<p>Copyright &copy; 2016 Christophe Guébert"
			   "<p>Panda is a framework for procedural drawing and animation."));
}

void MainWindow::openRecentFile()
{
	if (okToContinue())
	{
		QAction *action = qobject_cast<QAction *>(sender());
		if(action)
			loadFile(action->data().toString());
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
	if(m_openGLRenderView && m_openGLRenderView->isVisible())
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
	connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

	auto openAction = new QAction(tr("&Open..."), this);
	openAction->setIcon(QIcon(":/share/icons/open.png"));
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("Open an existing panda document"));
	connect(openAction, &QAction::triggered, this, &MainWindow::open);

	m_importAction = new QAction(tr("&Import..."), this);
	m_importAction->setIcon(QIcon(":/share/icons/open.png"));
	m_importAction->setStatusTip(tr("Import an existing panda document into the current one"));
	connect(m_importAction, &QAction::triggered, this, &MainWindow::import);

	auto saveAction = new QAction(tr("&Save"), this);
	saveAction->setIcon(QIcon(":/share/icons/save.png"));
	saveAction->setShortcut(QKeySequence::Save);
	saveAction->setStatusTip(tr("Save the document to disk"));
	connect(saveAction, &QAction::triggered, this, &MainWindow::save);

	auto saveAsAction = new QAction(tr("Save &As..."), this);
	saveAsAction->setShortcut(QKeySequence::SaveAs);
	saveAsAction->setStatusTip(tr("Save the document under a new "
								  "name"));
	connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], &QAction::triggered,
				this, &MainWindow::openRecentFile);
	}

	auto exitAction = new QAction(tr("E&xit"), this);
	exitAction->setShortcut(tr("Ctrl+Q"));
	exitAction->setStatusTip(tr("Exit Panda"));
	connect(exitAction, &QAction::triggered, this, &MainWindow::close);

	m_cutAction = new QAction(tr("Cu&t"), this);
	m_cutAction->setIcon(QIcon(":/share/icons/cut.png"));
	m_cutAction->setShortcut(QKeySequence::Cut);
	m_cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	connect(m_cutAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->cut(); });
	m_graphViewsActions.push_back(m_cutAction);

	m_copyAction = new QAction(tr("&Copy"), this);
	m_copyAction->setIcon(QIcon(":/share/icons/copy.png"));
	m_copyAction->setShortcut(QKeySequence::Copy);
	m_copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	connect(m_copyAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->copy(); });
	m_graphViewsActions.push_back(m_copyAction);

	m_pasteAction = new QAction(tr("&Paste"), this);
	m_pasteAction->setIcon(QIcon(":/share/icons/paste.png"));
	m_pasteAction->setShortcut(QKeySequence::Paste);
	m_pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	connect(m_pasteAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->paste(); });
	m_graphViewsActions.push_back(m_pasteAction);

	m_deleteAction = new QAction(tr("&Delete"), this);
	m_deleteAction->setShortcut(QKeySequence::Delete);
	m_deleteAction->setStatusTip(tr("Delete the current selection's contents"));
	connect(m_deleteAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->del(); });
	m_graphViewsActions.push_back(m_deleteAction);

	auto selectAllAction = new QAction(tr("Select &all"), this);
	selectAllAction->setShortcut(QKeySequence::SelectAll);
	selectAllAction->setStatusTip(tr("Select all objects"));
	connect(selectAllAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().selection().selectAll(); });
	m_graphViewsActions.push_back(selectAllAction);

	auto selectNoneAction = new QAction(tr("Select &none"), this);
	selectNoneAction->setShortcut(tr("Ctrl+Shift+A"));
	selectNoneAction->setStatusTip(tr("Deselect all objets"));
	connect(selectNoneAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().selection().selectNone(); });
	m_graphViewsActions.push_back(selectNoneAction);

	auto selectConnectedAction = new QAction(tr("Select &connected"), this);
	selectConnectedAction->setShortcut(tr("Ctrl+Shift+C"));
	selectConnectedAction->setStatusTip(tr("Select all objects connected to the current one"));
	connect(selectConnectedAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().selection().selectConnected(); });
	m_graphViewsActions.push_back(selectConnectedAction);

	m_groupAction = new QAction(tr("&Group selected"), this);
	m_groupAction->setShortcut(tr("Ctrl+G"));
	m_groupAction->setStatusTip(tr("Group selected objects"));
	m_groupAction->setEnabled(false);
	connect(m_groupAction, &QAction::triggered, this, &MainWindow::group);
	m_graphViewsActions.push_back(m_groupAction);

	m_ungroupAction = new QAction(tr("&Ungroup selected"), this);
	m_ungroupAction->setShortcut(tr("Ctrl+Shift+G"));
	m_ungroupAction->setStatusTip(tr("Ungroup selected objects"));
	m_ungroupAction->setEnabled(false);
	connect(m_ungroupAction, &QAction::triggered, this, &MainWindow::ungroup);
	m_graphViewsActions.push_back(m_ungroupAction);

	m_editGroupAction = new QAction(tr("&Edit group"), this);
	m_editGroupAction->setShortcut(tr("Ctrl+E"));
	m_editGroupAction->setStatusTip(tr("Edit the selected group name and parameters"));
	m_editGroupAction->setEnabled(false);
	connect(m_editGroupAction, &QAction::triggered, this, &MainWindow::editGroup);
	m_graphViewsActions.push_back(m_editGroupAction);

	m_openGroupAction = new QAction(tr("&Open group"), this);
	m_openGroupAction->setShortcut(tr("Ctrl+R"));
	m_openGroupAction->setStatusTip(tr("Open the group in a new window"));
	m_openGroupAction->setEnabled(false);
	connect(m_openGroupAction, &QAction::triggered, this, &MainWindow::openGroup);
	m_graphViewsActions.push_back(m_openGroupAction);

	m_saveGroupAction = new QAction(tr("&Save group"), this);
	m_saveGroupAction->setShortcut(tr("Ctrl+Shift+E"));
	m_saveGroupAction->setStatusTip(tr("Save the selected group for later use"));
	m_saveGroupAction->setEnabled(false);
	connect(m_saveGroupAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->saveGroup(); });
	m_graphViewsActions.push_back(m_saveGroupAction);

	auto zoomResetAction = new QAction(tr("Reset &zoom"), this);
	zoomResetAction->setShortcut(tr("Ctrl+0"));
	zoomResetAction->setStatusTip(tr("Set zoom to 100%"));
	zoomResetAction->setShortcutContext(Qt::WidgetShortcut);
	connect(zoomResetAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().viewport().zoomReset(); });
	m_allViewsActions.push_back(zoomResetAction);

	auto zoomInAction = new QAction(tr("Zoom &in"), this);
	zoomInAction->setShortcut(QKeySequence::ZoomIn);
	zoomInAction->setStatusTip(tr("Zoom in"));
	zoomInAction->setShortcutContext(Qt::WidgetShortcut);
	connect(zoomInAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().viewport().zoomIn(); });
	m_allViewsActions.push_back(zoomInAction);

	auto zoomOutAction = new QAction(tr("Zoom &out"), this);
	zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	zoomOutAction->setStatusTip(tr("Zoom out"));
	zoomOutAction->setShortcutContext(Qt::WidgetShortcut);
	connect(zoomOutAction,&QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().viewport().zoomOut(); });
	m_allViewsActions.push_back(zoomOutAction);

	auto centerViewAction = new QAction(tr("&Center view"), this);
	centerViewAction->setShortcut(tr("Ctrl+5"));
	centerViewAction->setStatusTip(tr("Center the view"));
	centerViewAction->setShortcutContext(Qt::WidgetShortcut);
	connect(centerViewAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().viewport().centerView(); });
	m_graphViewsActions.push_back(centerViewAction);

	auto showAllAction = new QAction(tr("Show &all"), this);
	showAllAction->setShortcut(tr("Ctrl+f"));
	showAllAction->setStatusTip(tr("Center and zoom the view so that all objects are visible"));
	showAllAction->setShortcutContext(Qt::WidgetShortcut);
	connect(showAllAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().viewport().showAll(); });
	m_graphViewsActions.push_back(showAllAction);

	auto showAllSelectedAction = new QAction(tr("Show all &selected"), this);
	showAllSelectedAction->setShortcut(tr("Ctrl+d"));
	showAllSelectedAction->setStatusTip(tr("Center and zoom the view so that all selected objects are visible"));
	showAllSelectedAction->setShortcutContext(Qt::WidgetShortcut);
	connect(showAllSelectedAction, &QAction::triggered, [this]() { if(m_currentGraphView) m_currentGraphView->view().viewport().showAllSelected(); });
	m_graphViewsActions.push_back(showAllSelectedAction);

	auto showGraphViewAction = new QAction(tr("Show &graph view"), this);
	showGraphViewAction->setShortcut(tr("Ctrl+1"));
	showGraphViewAction->setStatusTip(tr("Switch to the graph view"));
	connect(showGraphViewAction, &QAction::triggered, this, &MainWindow::switchToDocumentView);
	addAction(showGraphViewAction);

	auto showOpenGLViewAction = new QAction(tr("Show &render view"), this);
	showOpenGLViewAction->setShortcut(tr("Ctrl+2"));
	showOpenGLViewAction->setStatusTip(tr("Switch to the render view"));
	connect(showOpenGLViewAction, &QAction::triggered, this, &MainWindow::switchToOpenGLView);
	addAction(showOpenGLViewAction);

	auto adjustRenderSizeToViewAction = new QAction(tr("Adjust size to &view"), this);
	adjustRenderSizeToViewAction->setShortcut(tr("F10"));
	adjustRenderSizeToViewAction->setStatusTip(tr("Adjust the render size to the view"));
	adjustRenderSizeToViewAction->setCheckable(true);
	connect(adjustRenderSizeToViewAction, &QAction::triggered, this, &MainWindow::adjustRenderSizeToView);
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
	connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

	auto aboutQtAction = new QAction(tr("About &Qt"), this);
	aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

	m_playAction = new QAction(tr("Play"), this);
	m_playAction->setIcon(QIcon(":/share/icons/play.png"));
	m_playAction->setShortcut(tr("F5"));
	m_playAction->setStatusTip(tr("Start the animation"));
	m_playAction->setCheckable(true);
	connect(m_playAction, &QAction::triggered, this, &MainWindow::play);
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
	connect(m_removeLinkAction, &QAction::triggered, [this]() { if (m_currentGraphView) m_currentGraphView->view().interaction().removeLink(); });
	m_graphViewsActions.push_back(m_removeLinkAction);

	m_copyDataAction = new QAction(tr("Copy data"), this);
	m_copyDataAction->setStatusTip(tr("Create a user value generator based on this data"));
	connect(m_copyDataAction, &QAction::triggered, this, &MainWindow::copyDataToUserValue);
	m_graphViewsActions.push_back(m_copyDataAction);

	auto showLoggerDialogAction = new QAction(tr("Show &log"), this);
	showLoggerDialogAction->setShortcut(tr("F2"));
	showLoggerDialogAction->setStatusTip(tr("Show the updates log dialog"));
	connect(showLoggerDialogAction, &QAction::triggered, this, &MainWindow::showLoggerDialog);

	m_showDirtyInfoAction = new QAction(tr("Debug dirty state"), this);
	m_showDirtyInfoAction->setCheckable(true);
	m_showDirtyInfoAction->setStatusTip(tr("Show the dirty status of each object and data"));
	connect(m_showDirtyInfoAction, &QAction::triggered, [this](bool checked) { if (m_currentGraphView) m_currentGraphView->debugDirtyState(checked); });
	m_graphViewsActions.push_back(m_showDirtyInfoAction);

	auto showObjectsAndTypesAction = new QAction(tr("List types and objects"), this);
	showObjectsAndTypesAction->setStatusTip(tr("Show information about all available types and objects"));
	connect(showObjectsAndTypesAction, &QAction::triggered, this, &MainWindow::showObjectsAndTypes);

	m_showImageViewportAction = new QAction(tr("Open image viewport"), this);
	m_showImageViewportAction->setStatusTip(tr("Open a new viewport to an image"));
	connect(m_showImageViewportAction, &QAction::triggered, this, &MainWindow::showImageViewport);
	addAction(m_showImageViewportAction);

	m_setDataLabelAction = new QAction(tr("Set data label"), this);
	m_setDataLabelAction->setStatusTip(tr("Set a label for the data"));
	connect(m_setDataLabelAction, &QAction::triggered, [this]() { if (m_currentGraphView) m_currentGraphView->setDataLabel(); });
	m_graphViewsActions.push_back(m_setDataLabelAction);

	m_chooseWidgetAction = new QAction(tr("Choose widget"), this);
	m_chooseWidgetAction->setStatusTip(tr("Choose the widget to use for this data"));
	connect(m_chooseWidgetAction, &QAction::triggered, [this]() { if (m_currentGraphView) m_currentGraphView->showChooseWidgetDialog(); });
	m_graphViewsActions.push_back(m_chooseWidgetAction);

	m_objectToBackAction = new QAction(tr("Move object to back"), this);
	m_objectToBackAction->setStatusTip(tr("Move the selected object to the back of the others"));
	connect(m_objectToBackAction, &QAction::triggered, [this]() { if (m_currentGraphView) m_currentGraphView->view().interaction().moveObjectToBack(); });
	m_graphViewsActions.push_back(m_objectToBackAction);

	m_objectToFrontAction = new QAction(tr("Move object to front"), this);
	m_objectToFrontAction->setStatusTip(tr("Move the selected object in front of the others"));
	connect(m_objectToFrontAction, &QAction::triggered, [this]() { if (m_currentGraphView) m_currentGraphView->view().interaction().moveObjectToFront(); });
	m_graphViewsActions.push_back(m_objectToFrontAction);

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
	connect(convertDocumentsAction, &QAction::triggered, this, &MainWindow::convertSavedDocuments);

	// Alignment (horizontal)
	auto alignHorCenterAction = new QAction(tr("Center"), this);
	alignHorCenterAction->setIcon(QIcon(":/share/icons/align-horizontal-center.png"));
	alignHorCenterAction->setStatusTip(tr("Center on vertical axis"));
	connect(alignHorCenterAction, &QAction::triggered, [this]() { if(m_currentGraphView) alignHorizontallyCenter(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(alignHorCenterAction);

	auto alignHorLeftAction = new QAction(tr("Left"), this);
	alignHorLeftAction->setIcon(QIcon(":/share/icons/align-horizontal-left.png"));
	alignHorLeftAction->setStatusTip(tr("Align left edges"));
	connect(alignHorLeftAction, &QAction::triggered, [this]() { if(m_currentGraphView) alignHorizontallyLeft(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(alignHorLeftAction);

	auto alignHorRightAction = new QAction(tr("Right"), this);
	alignHorRightAction->setIcon(QIcon(":/share/icons/align-horizontal-right.png"));
	alignHorRightAction->setStatusTip(tr("Align right edges"));
	connect(alignHorRightAction, &QAction::triggered, [this]() { if(m_currentGraphView) alignHorizontallyRight(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(alignHorRightAction);

	// Alignment (vertical)
	auto alignVertCenterAction = new QAction(tr("Center"), this);
	alignVertCenterAction->setIcon(QIcon(":/share/icons/align-vertical-center.png"));
	alignVertCenterAction->setStatusTip(tr("Center on horizontal axis"));
	connect(alignVertCenterAction, &QAction::triggered, [this]() { if(m_currentGraphView) alignVerticallyCenter(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(alignVertCenterAction);

	auto alignVertTopAction = new QAction(tr("Top"), this);
	alignVertTopAction->setIcon(QIcon(":/share/icons/align-vertical-top.png"));
	alignVertTopAction->setStatusTip(tr("Align top edges"));
	connect(alignVertTopAction, &QAction::triggered, [this]() { if(m_currentGraphView) alignVerticallyTop(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(alignVertTopAction);

	auto alignVertBottomAction = new QAction(tr("Bottom"), this);
	alignVertBottomAction->setIcon(QIcon(":/share/icons/align-vertical-bottom.png"));
	alignVertBottomAction->setStatusTip(tr("Align bottom edges"));
	connect(alignVertBottomAction, &QAction::triggered, [this]() { if(m_currentGraphView) alignVerticallyBottom(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(alignVertBottomAction);

	// Distribution (horizontal)
	auto distributeHorCenterAction = new QAction(tr("Center"), this);
	distributeHorCenterAction->setIcon(QIcon(":/share/icons/distribute-horizontal-center.png"));
	distributeHorCenterAction->setStatusTip(tr("Distribute centers equidistantly horizontally"));
	connect(distributeHorCenterAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeHorizontallyCenter(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeHorCenterAction);

	auto distributeHorGapsAction = new QAction(tr("Gaps"), this);
	distributeHorGapsAction->setIcon(QIcon(":/share/icons/distribute-horizontal-gaps.png"));
	distributeHorGapsAction->setStatusTip(tr("Make horizontal gaps between objects equal"));
	connect(distributeHorGapsAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeHorizontallyGaps(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeHorGapsAction);

	auto distributeHorLeftAction = new QAction(tr("Left"), this);
	distributeHorLeftAction->setIcon(QIcon(":/share/icons/distribute-horizontal-left.png"));
	distributeHorLeftAction->setStatusTip(tr("Distribute left edges equidistantly"));
	connect(distributeHorLeftAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeHorizontallyLeft(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeHorLeftAction);

	auto distributeHorRightAction = new QAction(tr("Right"), this);
	distributeHorRightAction->setIcon(QIcon(":/share/icons/distribute-horizontal-right.png"));
	distributeHorRightAction->setStatusTip(tr("Distribute left edges equidistantly"));
	connect(distributeHorRightAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeHorizontallyRight(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeHorRightAction);

	// Distribution (vertical)
	auto distributeVertCenterAction = new QAction(tr("Center"), this);
	distributeVertCenterAction->setIcon(QIcon(":/share/icons/distribute-vertical-center.png"));
	distributeVertCenterAction->setStatusTip(tr("Distribute centers equidistantly vertically"));
	connect(distributeVertCenterAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeVerticallyCenter(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeVertCenterAction);

	auto distributeVertGapsAction = new QAction(tr("Gaps"), this);
	distributeVertGapsAction->setIcon(QIcon(":/share/icons/distribute-vertical-gaps.png"));
	distributeVertGapsAction->setStatusTip(tr("Make vertical gaps between objects equal"));
	connect(distributeVertGapsAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeVerticallyGaps(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeVertGapsAction);

	auto distributeVertTopAction = new QAction(tr("Top"), this);
	distributeVertTopAction->setIcon(QIcon(":/share/icons/distribute-vertical-top.png"));
	distributeVertTopAction->setStatusTip(tr("Distribute top edges equidistantly"));
	connect(distributeVertTopAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeVerticallyTop(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeVertTopAction);

	auto distributeVertBottomAction = new QAction(tr("Bottom"), this);
	distributeVertBottomAction->setIcon(QIcon(":/share/icons/distribute-vertical-bottom.png"));
	distributeVertBottomAction->setStatusTip(tr("Distribute bottom edges equidistantly"));
	connect(distributeVertBottomAction, &QAction::triggered, [this]() { if(m_currentGraphView) distributeVerticallyBottom(m_currentGraphView->view()); });
	m_graphViewsActions.push_back(distributeVertBottomAction);

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
	m_groupMenu->addAction(m_openGroupAction);
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
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(m_showDirtyInfoAction);
#ifdef PANDA_LOG_EVENTS
	m_viewMenu->addAction(showLoggerDialogAction);
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
	typedef std::map<QString, QAction*> ActionsMap;
	typedef std::map<QString, menuItemInfo> ChildsMap;

	ActionsMap actions;
	ChildsMap childs;

	void registerActions(QMenu* menu)
	{
		for(const auto& action : actions)
			menu->addAction(action.second);

		for(auto& child : childs)
			child.second.registerActions(menu->addMenu(child.first));
	}
};

void MainWindow::createRegistryMenu()
{
	const auto& registryMap = panda::ObjectFactory::registryMap();
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

			connect(tempAction, &QAction::triggered, this, &MainWindow::createObject);
		}

		menuTree.registerActions(m_registryMenu);
	}
}

void MainWindow::createGroupRegistryMenu()
{
	if(m_groupsRegistryMenu)
		m_groupsRegistryMenu->clear();
	panda::GroupsManager::createGroupsList();

	const auto& groups = panda::GroupsManager::groups();
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
			const QString& display = QString::fromStdString(group.first);
			QStringList hierarchy = display.split("/");
			menuItemInfo* currentMenu = &menuTree;
			for(int i=0; i<hierarchy.count()-1; ++i)
				currentMenu = &currentMenu->childs[hierarchy[i]];

			QAction* tempAction = new QAction(hierarchy.last(), this);
			tempAction->setStatusTip(QString::fromStdString(group.second.description));
			tempAction->setData(display);
			currentMenu->actions[hierarchy.last()] = tempAction;

			connect(tempAction, &QAction::triggered, this, &MainWindow::createGroupObject);
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

bool MainWindow::loadFile(const QString &fileName)
{
	std::unique_ptr<panda::PandaDocument> document;

	try 
	{
		document = panda::serialization::readFile(fileName.toStdString(), *m_simpleGUI);
	} 
	catch (const std::exception& e)
	{
		QMessageBox::warning(this, "Load error", QString::fromLocal8Bit(e.what()));
		return false;
	}

	setDocument(std::move(document));

	m_playAction->setChecked(false);

	m_documentView->view().selection().set(panda::graph::getRawObjectsList(m_document->getObjectsList().get()));

	m_document->getUndoStack().clear();
	m_documentView->view().selection().selectNone();
	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);

	m_documentView->executeNextRefresh([view = m_documentView] { view->view().viewport().showAll(); });
	return true;
}

bool MainWindow::importFile(const QString& fileName)
{
	panda::serialization::Objects result;
	try 
	{
		result = panda::serialization::importFile(m_document.get(), m_document->getObjectsList(), fileName.toStdString());
	} 
	catch (const std::exception& e)
	{
		QMessageBox::warning(this, "Import error", QString::fromLocal8Bit(e.what()));
		return false;
	}

	m_documentView->view().selection().set(result);

	statusBar()->showMessage(tr("File imported"), 2000);
	m_documentView->view().viewport().showAll();
	return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
	try
	{
		panda::serialization::writeFile(m_document.get(), fileName.toStdString());
	}
	catch(const std::exception& e)
	{
		QMessageBox::warning(this, "Save error", QString::fromLocal8Bit(e.what()));
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
		auto object = panda::ObjectFactory::create(action->data().toString().toStdString(), m_document.get());
		auto& objectsList = m_currentGraphView ? m_currentGraphView->view().objectsList() : m_document->getObjectsList();
		m_document->getUndoStack().push(std::make_shared<panda::AddObjectCommand>(m_document.get(), objectsList, object));
	}
}

void MainWindow::switchToDocumentView()
{
	m_tabWidget->setCurrentWidget(m_documentViewContainer);
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

void MainWindow::group()
{
	bool result = false;
	try
	{
		result = panda::createGroup(m_document.get(), &m_currentGraphView->view());
	}
	catch (const std::exception& e)
	{
		QMessageBox::warning(this, "Group error", QString::fromLocal8Bit(e.what()));
	}	

	if(!result)
		statusBar()->showMessage(tr("Could not create a group from the selection"), 2000);
}

void MainWindow::ungroup()
{
	bool result = false;
	try
	{
		panda::ungroupSelection(m_document.get(), &m_currentGraphView->view());
	}
	catch (const std::exception& e)
	{
		QMessageBox::warning(this, "Unroup error", QString::fromLocal8Bit(e.what()));
	}

	if(!result)
		statusBar()->showMessage(tr("Could not ungroup the selection"), 2000);
}

void MainWindow::editGroup()
{
	panda::Group* group = dynamic_cast<panda::Group*>(m_currentGraphView->view().selection().lastSelectedObject());
	if(group)
	{
		EditGroupDialog dlg(group, this);
		dlg.exec();
	}
	else
		statusBar()->showMessage(tr("The selected objet is not a group"), 2000);
}

void MainWindow::createGroupObject()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if(action)
	{
		const auto path = action->data().toString().toStdString();
		panda::GroupsManager::createGroupObject(m_document.get(), &m_currentGraphView->view(), path);
	}
}

void MainWindow::openGroup()
{
	const auto& selection = m_currentGraphView->view().selection().get();
	if (selection.size() != 1)
		return;

	auto group = dynamic_cast<panda::Group*>(selection.front());
	if (!group)
		return;

	auto it = std::find_if(m_graphViews.begin(), m_graphViews.end(), [group](const GraphViewInfo& info) {
		return info.object == group;
	});
	if (it != m_graphViews.end()) // Do not open another view
	{
		m_tabWidget->setCurrentWidget(it->container);
		it->container->setFocus();
		return;
	}

	auto view = panda::graphview::DocumentDatasView::createDocumentDatasView(group->groupDatas(), m_document.get(), group->getObjectsList());
	auto groupView = new graphview::QtViewWrapper(std::move(view), this);

	connect(groupView, &graphview::QtViewWrapper::modified, this, &MainWindow::documentModified);
	connect(groupView, &graphview::QtViewWrapper::lostFocus, this, &MainWindow::onTabWidgetFocusLoss);
	m_observer.get(groupView->view().selection().selectedObject).connect<MainWindow, &MainWindow::selectedObject>(this);
	m_observer.get(group->getObjectsList().removedObject).connect<MainWindow, &MainWindow::removedObject>(this);
	
	// Register the actions for the group view
	for (auto action : m_allViewsActions)
		groupView->addAction(action);
	for (auto action : m_graphViewsActions)
		groupView->addAction(action);

	auto groupViewContainer = new ScrollContainer();
	groupViewContainer->setFrameStyle(0); // No frame
	groupViewContainer->setView(groupView);
	groupViewContainer->setFocusProxy(groupView);

	auto detachableInfo = new DetachableWidgetInfo(groupViewContainer);

	auto getLabel = [group, detachableInfo]() {
		const auto& name = group->getName();
		const auto& label = group->getLabel();
		std::string tabName = name;
		if (!label.empty())
			tabName = label + " (" + name + ")";
		detachableInfo->changeTitle(QString::fromStdString(tabName));
		return tabName;
	};

	GraphViewInfo info;
	info.view = groupView;
	info.container = groupViewContainer;
	info.detachableInfo = detachableInfo;
	info.object = group;
	info.nameWatcher = std::make_shared<DataWatcher>(group->getGroupNameData(), getLabel);
	m_graphViews.push_back(info);

	m_tabWidget->addTab(groupViewContainer, QString::fromStdString(getLabel()), detachableInfo);

	m_tabWidget->setCurrentWidget(groupViewContainer);
	groupViewContainer->setFocus();
	groupView->view().viewport().showAll();
}

void MainWindow::fillContextMenu(QMenu& menu, int typesVal) const
{
	const auto types = static_cast<panda::graphview::MenuTypes>(typesVal);
	using mt = panda::graphview::MenuType;
	panda::PandaObject* obj = m_currentGraphView->view().selection().lastSelectedObject();
	if(obj)
	{
		menu.addAction(m_cutAction);
		menu.addAction(m_copyAction);
	}
	menu.addAction(m_pasteAction);

	if(types & mt::Link)
		menu.addAction(m_removeLinkAction);

	if(types & mt::Data)
	{
		menu.addAction(m_copyDataAction);
		const panda::PandaObject* owner = nullptr;
		auto data = m_currentGraphView->view().interaction().contextMenuData();
		if(data)
			owner = data->getOwner();
		if(owner && owner->getClass()->getClassName() == "GeneratorUser" && owner->getClass()->getNamespaceName() == "panda")
			menu.addAction(m_chooseWidgetAction);

		if (data && !panda::VisualizersManager::visualizers(data->getDataTrait()->fullTypeId()).empty())
			menu.addAction(tr("Visualize"), m_currentGraphView, &graphview::QtViewWrapper::createVisualizer);
	}

	if(obj && obj->getClass()->getClassName() == "GeneratorUser" && obj->getClass()->getNamespaceName() == "panda")
		menu.addAction(m_chooseWidgetAction);

	if(types & mt::Image)
		menu.addAction(m_showImageViewportAction);

	if (types & mt::Tag || types & mt::Data)
		menu.addAction(m_setDataLabelAction);

	if (types & mt::Selection)
	{
		int nbSelected = m_currentGraphView->view().selection().get().size();
		if (nbSelected == 1 && dynamic_cast<panda::Annotation*>(obj))
		{
			menu.addSeparator();
			menu.addAction(m_objectToFrontAction);
			menu.addAction(m_objectToBackAction);
		}

		if (nbSelected == 1 && dynamic_cast<panda::Group*>(obj))
		{
			menu.addSeparator();
			menu.addAction(m_ungroupAction);
			menu.addAction(m_editGroupAction);
			menu.addAction(m_openGroupAction);
			menu.addAction(m_saveGroupAction);
		}

		if (nbSelected > 1)
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
	}
}

void MainWindow::copyDataToUserValue()
{
	const panda::BaseData* clickedData = m_currentGraphView->view().interaction().contextMenuData();
	if(clickedData)
		panda::copyDataToUserValue(clickedData, m_document.get(), m_currentGraphView->view().objectsList());
}

void MainWindow::showLoggerDialog()
{
	if(!m_loggerDialog)
	{
		m_loggerDialog = new UpdateLoggerDialog(this);
		UpdateLoggerDialog::setInstance(m_loggerDialog);

		connect(m_loggerDialog, &UpdateLoggerDialog::changedSelectedEvent, [view = m_documentView] { view->update(); });
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

	m_document->play(playing);
}

void MainWindow::selectedObject(panda::PandaObject* object)
{
	int nbSelected = m_currentGraphView ? m_currentGraphView->view().selection().get().size() : 0;
	bool isGroup = (nbSelected == 1) && dynamic_cast<panda::Group*>(object);

	m_ungroupAction->setEnabled(isGroup);
	m_editGroupAction->setEnabled(isGroup);
	m_openGroupAction->setEnabled(isGroup);
	m_saveGroupAction->setEnabled(isGroup);

	m_groupAction->setEnabled(nbSelected > 1);

	m_datasTable->setSelectedObject(object);
}

void MainWindow::showImageViewport()
{
	const panda::BaseData* clickedData = m_currentGraphView->view().interaction().contextMenuData();
	if (!clickedData)
		return;

	auto trait = clickedData->getDataTrait();

	if (trait->isSingleValue())
	{
		auto it = std::find_if(m_imageViewports.begin(), m_imageViewports.end(), [clickedData](const ImageViewportInfo& info) {
			return info.data == clickedData;
		});
		if (it != m_imageViewports.end()) // Do not open another viewport
		{
			m_tabWidget->setCurrentWidget(it->container);
			it->container->setFocus();
			return;
		}
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
	connect(imageViewport, &ImageViewport::closeViewport, this, &MainWindow::closeViewport);
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

	m_tabWidget->setCurrentWidget(container);
	container->setFocus();
}

void MainWindow::openDetachedWindow(DetachedWindow* window)
{
	m_detachedWindows.push_back(window);
	connect(window, &DetachedWindow::closeDetachedWindow, this, &MainWindow::closeDetachedWindow);
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
	if (it != m_imageViewports.end())
		closeTab(it->container);
}

void MainWindow::closeGraphView(graphview::QtViewWrapper* view)
{
	auto it = std::find_if(m_graphViews.begin(), m_graphViews.end(), [view](const GraphViewInfo& info) {
		return info.view == view;
	});
	if (it != m_graphViews.end())
		closeTab(it->container);
}

void MainWindow::closeTab(QWidget* container)
{
	for(DetachedWindow* window : m_detachedWindows)
	{
		if(window->getTabInfo().widget == container)
		{
			window->closeTab();
			onTabWidgetCloseTab(container); // To be sure we remove the tab info from the lists
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

void MainWindow::removedObject(panda::PandaObject* object)
{
	// Close the image viewports connected to it
	auto imageViewports = m_imageViewports;
	for (const auto& info : imageViewports)
	{
		if (info.object == object)
			closeViewport(info.viewport);
	}

	// Close any graph view opened
	auto graphViews = m_graphViews;
	for (const auto& info : graphViews)
	{
		if (info.object == object)
			closeGraphView(info.view);
	}
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
		
		try
		{
			auto document = panda::serialization::readFile(path.toStdString(), *m_simpleGUI);
		
			panda::serialization::writeFile(document.get(), path.toStdString());
			++nb;
		}
		catch(const std::exception&)
		{
			if (QMessageBox::Abort == QMessageBox::question(this, 
				tr("Error loading document"), 
				tr("Could not open %1").arg(entry.fileName()), 
				QMessageBox::Abort | QMessageBox::Ignore))
				break;
		}
	}

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

void MainWindow::onTabWidgetCloseTab(QWidget* w)
{
	// Remove corresponding image viewport if any
	auto it = std::find_if(m_imageViewports.begin(), m_imageViewports.end(), [w](const ImageViewportInfo& info) {
		return info.container == w;
	});
	if (it != m_imageViewports.end())
		m_imageViewports.erase(it);

	// Remove corresponding graph view if any
	auto it2 = std::find_if(m_graphViews.begin(), m_graphViews.end(), [w](const GraphViewInfo& info) {
		return info.container == w;
	});
	if (it2 != m_graphViews.end())
		m_graphViews.erase(it2);
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

	if (hasParent(m_documentView, currentTabWidget))
		return m_documentView;

	for (const auto& imageViewport : m_imageViewports)
	{
		if (hasParent(imageViewport.viewport, currentTabWidget))
			return imageViewport.viewport;
	}

	for (const auto& graphView : m_graphViews)
	{
		if (hasParent(graphView.view, currentTabWidget))
			return graphView.view;
	}

	return nullptr;
}

void MainWindow::onTabChanged()
{
	auto widget = selectedTabWidget();
	m_currentGraphView = dynamic_cast<graphview::QtViewWrapper*>(widget);
	
	bool isAnyView = (widget != m_openGLRenderView);
	for (auto action : m_allViewsActions)
		action->setEnabled(isAnyView);

	bool isGraphView = (m_currentGraphView != nullptr);
	for (auto action : m_graphViewsActions)
		action->setEnabled(isGraphView);

	if (m_currentGraphView)
		selectedObject(m_currentGraphView->view().selection().lastSelectedObject());
	else
		m_currentGraphView = m_documentView;
}

void MainWindow::setDocument(const std::shared_ptr<panda::PandaDocument>& document)
{
	m_document = document;

	if (m_documentView)
		m_documentView->deleteLater();

	std::unique_ptr<panda::graphview::GraphView> view;
	auto visualizerDocument = std::dynamic_pointer_cast<panda::VisualizerDocument>(document);
	if (visualizerDocument)
		view = panda::graphview::DocumentDatasView::createDocumentDatasView(visualizerDocument->documentDatas(), m_document.get(), m_document->getObjectsList());
	else
		view = panda::graphview::DocumentView::createDocumentView(m_document.get(), m_document->getObjectsList());
	m_documentView = new graphview::QtViewWrapper(std::move(view), this);
	m_documentViewContainer->setView(m_documentView);
	m_documentViewContainer->setFocusProxy(m_documentView);
	m_currentGraphView = m_documentView;

	if (m_openGLRenderView)
	{
		m_openGLRenderView->deleteLater();
		m_openGLRenderView = nullptr;
	}

	int tabIndex = m_tabWidget->indexOf(m_openGLViewContainer);
	auto renderedDocument = dynamic_cast<panda::RenderedDocument*>(document.get());
	if (renderedDocument)
	{
		m_openGLRenderView = new OpenGLRenderView(renderedDocument);
		m_openGLViewContainer->setWidget(m_openGLRenderView);
		m_openGLViewContainer->setFocusProxy(m_openGLRenderView);

		if (tabIndex == -1)
			m_tabWidget->addTab(m_openGLViewContainer, tr("Render"));

		m_tabWidget->setCurrentWidget(m_openGLViewContainer); // First go to the OpenGL view (initialize it)
	
		// Go back to the graph view asap
		QTimer::singleShot(0, [this](){
			m_tabWidget->setCurrentWidget(m_documentViewContainer);
			m_documentView->setFocus();
		});

		connect(m_openGLRenderView, &OpenGLRenderView::lostFocus, this, &MainWindow::onTabWidgetFocusLoss);
	}
	else
	{
		if (tabIndex != -1)
			m_tabWidget->removeTab(tabIndex);
	}

	m_observer.get(m_document->getSignals().modified).connect<MainWindow, &MainWindow::documentModified>(this);
	m_observer.get(m_document->getObjectsList().removedObject).connect<MainWindow, &MainWindow::removedObject>(this);
	m_observer.get(m_documentView->view().selection().selectedObject).connect<MainWindow, &MainWindow::selectedObject>(this);
	m_observer.get(m_documentView->view().modified).connect<MainWindow, &MainWindow::documentModified>(this);

	m_observer.get(m_document->getUndoStack().m_canUndoChangedSignal).connect<MainWindow, &MainWindow::undoEnabled>(this);
	m_observer.get(m_document->getUndoStack().m_canRedoChangedSignal).connect<MainWindow, &MainWindow::redoEnabled>(this);
	m_observer.get(m_document->getUndoStack().m_undoTextChangedSignal).connect<MainWindow, &MainWindow::undoTextChanged>(this);
	m_observer.get(m_document->getUndoStack().m_redoTextChangedSignal).connect<MainWindow, &MainWindow::redoTextChanged>(this);

	connect(m_documentView, &graphview::QtViewWrapper::modified, this, &MainWindow::documentModified);
	connect(m_documentView, &graphview::QtViewWrapper::lostFocus, this, &MainWindow::onTabWidgetFocusLoss);
	connect(m_documentView, &graphview::QtViewWrapper::groupsListModified, this, &MainWindow::createGroupRegistryMenu);
	
	m_datasTable->setDocument(m_document);
	m_datasTable->setSelectedObject(m_document.get());

	m_layersTab->setDocument(m_document);

	for (auto action : m_allViewsActions)
		m_documentView->addAction(action);
	for (auto action : m_graphViewsActions)
		m_documentView->addAction(action);

	m_observer.get(m_document->getSignals().timeChanged).connect<MainWindow, &MainWindow::updateStatusBar>(this);
	updateStatusBar();

	// Enable actions depending on if the corresponding object can be created or not with this new document
	updateAddObjectActions(m_registryMenu);
	updateAddGroupActions(m_groupsRegistryMenu);

	m_documentView->executeNextRefresh([view = m_documentView] { view->view().viewport().showAll(); });
}

void MainWindow::updateAddObjectActions(QMenu* menu)
{
	for (auto action : menu->actions())
	{
		if (action->isSeparator())
			continue;
		if (action->menu())
		{
			updateAddObjectActions(action->menu());
			continue;
		}

		action->setEnabled(panda::ObjectFactory::canCreate(action->data().toString().toStdString(), m_document.get()));
	}
}

void MainWindow::updateAddGroupActions(QMenu* menu)
{
	if (!menu)
		return;

	auto docType = panda::serialization::getDocumentType(m_document.get());
	for (auto action : menu->actions())
	{
		if (action->isSeparator())
			continue;
		if (action->menu())
		{
			updateAddGroupActions(action->menu());
			continue;
		}

		auto canCreate = panda::GroupsManager::canCreate(action->data().toString().toStdString(), docType);
		action->setEnabled(canCreate);
	}
}
