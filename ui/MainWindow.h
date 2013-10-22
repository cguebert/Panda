#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QLabel;
class GraphView;
class RenderView;
class QDockWidget;
class DatasTable;
class LayersTab;
class UpdateLoggerDialog;

namespace panda
{
class PandaDocument;
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
	void switchToRenderView();
	void showStatusBarMessage(QString);
	void paste();
	void group();
	void ungroup();
	void editGroup();
	void saveGroup();
	void createGroupObject();
	void showContextMenu(QPoint, int);
	void copyDataToUserValue();
	void showLoggerDialog();

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

	QTabWidget* tabWidget;
	GraphView* graphView;
	RenderView* renderView;
	panda::PandaDocument* pandaDocument;
	DatasTable* datasTable;
	QDockWidget* datasDock;
	LayersTab* layersTab;
	QDockWidget* layersDock;
	QStringList recentFiles;
	QString curFile;
	UpdateLoggerDialog* loggerDialog;

	enum { MaxRecentFiles = 5 };
	QAction* recentFileActions[MaxRecentFiles];
	QAction* separatorAction;

	QMenu* fileMenu;
	QMenu* editMenu;
	QMenu* selectMenu;
	QMenu* groupMenu;
	QMenu* viewMenu;
	QMenu* registryMenu;
	QMenu* toolsMenu;
	QMenu* optionsMenu;
	QMenu* helpMenu;
	QMenu* groupsRegistryMenu;
	QToolBar* fileToolBar;
	QToolBar* editToolBar;
	QToolBar* animToolBar;
	QAction* newAction;
	QAction* openAction;
	QAction* importAction;
	QAction* saveAction;
	QAction* saveAsAction;
	QAction* exitAction;
	QAction* cutAction;
	QAction* copyAction;
	QAction* pasteAction;
	QAction* deleteAction;
	QAction* selectAllAction;
	QAction* selectNoneAction;
	QAction* selectConnectedAction;
	QAction* zoomResetAction;
	QAction* zoomInAction;
	QAction* zoomOutAction;
	QAction* centerViewAction;
	QAction* showAllAction;
	QAction* showAllSelectedAction;
	QAction* showGraphView;
	QAction* showRenderView;
	QAction* aboutAction;
	QAction* aboutQtAction;
	QAction* playAction;
	QAction* stepAction;
	QAction* rewindAction;
	QAction* groupAction;
	QAction* ungroupAction;
	QAction* editGroupAction;
	QAction* saveGroupAction;
	QAction* removeLinkAction;
	QAction* copyDataAction;
	QAction* showLoggerDialogAction;
	QLabel* timeLabel;
};

#endif
