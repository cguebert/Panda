#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QLabel;
class GraphView;
class OpenGLRenderView;
class QDockWidget;
class DatasTable;
class LayersTab;
class UpdateLoggerDialog;

namespace panda
{
class PandaDocument;
class PandaObject;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void newFile();
	void open();
	void import();
	bool save();
	bool saveAs();
	void about();
	void openRecentFile();
	void updateStatusBar();
	void documentModified();
	void createObject();
	void switchToGraphView();
	void switchToOpenGLView();
	void showStatusBarMessage(QString);
	void cut();
	void paste();
	void del();
	void group();
	void ungroup();
	void editGroup();
	void saveGroup();
	void createGroupObject();
	void showContextMenu(QPoint, int);
	void copyDataToUserValue();
	void showLoggerDialog();
	void showObjectsAndTypes();
	void play(bool);
	void selectedObject(panda::PandaObject*);

private:
	void createActions();
	void createMenus();
	void createRegistryMenu();
	void createGroupRegistryMenu();
	void createToolBars();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	bool okToContinue();
	bool loadFile(const QString &fileName, bool import = false);
	bool saveFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();
	QString strippedName(const QString &fullFileName);

	QTabWidget* m_tabWidget;
	GraphView* m_graphView;
	OpenGLRenderView* m_openGLRenderView;
	panda::PandaDocument* m_document;
	DatasTable* m_datasTable;
	QDockWidget* m_datasDock;
	LayersTab* m_layersTab;
	QDockWidget* m_layersDock;
	QStringList m_recentFiles;
	QString m_curFile;
	UpdateLoggerDialog* m_loggerDialog;

	enum { MaxRecentFiles = 5 };
	QAction* m_recentFileActions[MaxRecentFiles];
	QAction* m_separatorAction;

	QMenu* m_fileMenu;
	QMenu* m_editMenu;
	QMenu* m_selectMenu;
	QMenu* m_groupMenu;
	QMenu* m_viewMenu;
	QMenu* m_registryMenu;
	QMenu* m_toolsMenu;
	QMenu* m_optionsMenu;
	QMenu* m_helpMenu;
	QMenu* m_groupsRegistryMenu;
	QToolBar* m_fileToolBar;
	QToolBar* m_editToolBar;
	QToolBar* m_animToolBar;
	QAction* m_newAction;
	QAction* m_openAction;
	QAction* m_importAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_exitAction;
	QAction* m_cutAction;
	QAction* m_copyAction;
	QAction* m_pasteAction;
	QAction* m_deleteAction;
	QAction* m_selectAllAction;
	QAction* m_selectNoneAction;
	QAction* m_selectConnectedAction;
	QAction* m_zoomResetAction;
	QAction* m_zoomInAction;
	QAction* m_zoomOutAction;
	QAction* m_centerViewAction;
	QAction* m_showAllAction;
	QAction* m_showAllSelectedAction;
	QAction* m_showGraphView;
	QAction* m_showOpenGLView;
	QAction* m_aboutAction;
	QAction* m_aboutQtAction;
	QAction* m_playAction;
	QAction* m_stepAction;
	QAction* m_rewindAction;
	QAction* m_groupAction;
	QAction* m_ungroupAction;
	QAction* m_editGroupAction;
	QAction* m_saveGroupAction;
	QAction* m_removeLinkAction;
	QAction* m_copyDataAction;
	QAction* m_showLoggerDialogAction;
	QAction* m_showObjectsAndTypesAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	QLabel* m_timeLabel;
};

#endif
