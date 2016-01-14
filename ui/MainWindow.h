#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <panda/messaging.h>
#include <map>

class QAction;
class QDockWidget;
class QLabel;
class QScrollArea;

class DatasTable;
class DetachableTabWidget;
class DetachedWindow;
class GraphView;
class ImageViewport;
class LayersTab;
class OpenGLRenderView;
class SimpleGUIImpl;
class ScrollContainer;
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

	void updateOpenGLView() const;

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
	void switchFullScreen();
	void showStatusBarMessage(QString);
	void copy();
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
	void adjustRenderSizeToView();
	void showImageViewport();
	void openDetachedWindow(DetachedWindow* window);
	void closeDetachedWindow(DetachedWindow* window);
	void closeViewport(ImageViewport* viewport);
	void convertSavedDocuments();

private:
	void createActions();
	void createRegistryMenu();
	void createGroupRegistryMenu();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	bool okToContinue();
	bool loadFile(const QString &fileName, bool import = false);
	bool saveFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();
	QString strippedName(const QString &fullFileName);

	void undoEnabled(bool enabled);
	void redoEnabled(bool enabled);
	void undoTextChanged(const std::string& text);
	void redoTextChanged(const std::string& text);

	DetachableTabWidget* m_tabWidget = nullptr;
	GraphView* m_graphView = nullptr;
	ScrollContainer* m_graphViewContainer = nullptr;
	OpenGLRenderView* m_openGLRenderView = nullptr;
	QScrollArea* m_openGLViewContainer = nullptr;
	panda::PandaDocument* m_document = nullptr;
	DatasTable* m_datasTable = nullptr;
	QDockWidget* m_datasDock = nullptr;
	LayersTab* m_layersTab = nullptr;
	QDockWidget* m_layersDock = nullptr;
	UpdateLoggerDialog* m_loggerDialog = nullptr;
	SimpleGUIImpl* m_simpleGUI = nullptr;
	QStringList m_recentFiles;
	QString m_curFile;
	bool m_fullScreen = false, m_adjustRenderSizeToView = false, m_undoEnabled = false, m_redoEnabled = false;
	QList<DetachedWindow*> m_detachedWindows;
	std::map<ImageViewport*, QWidget*> m_imageViewports;
	panda::msg::Observer m_observer;

	enum { MaxRecentFiles = 5 };
	QAction* m_recentFileActions[MaxRecentFiles];
	QAction* m_separatorAction;

	QMenu *m_fileMenu,
		*m_editMenu,
		*m_selectMenu,
		*m_groupMenu,
		*m_viewMenu,
		*m_helpMenu,
		*m_registryMenu,
		*m_groupsRegistryMenu = nullptr,
		*m_alignHorizontallyMenu,
		*m_alignVerticallyMenu,
		*m_distributeHorizontallyMenu,
		*m_distributeVerticallyMenu;
	QToolBar *m_fileToolBar,
		*m_editToolBar,
		*m_animToolBar;
	QAction *m_importAction,
		*m_cutAction,
		*m_copyAction,
		*m_pasteAction,
		*m_deleteAction,
		*m_fullScreenAction,
		*m_playAction,
		*m_groupAction,
		*m_ungroupAction,
		*m_editGroupAction,
		*m_saveGroupAction,
		*m_removeLinkAction,
		*m_copyDataAction,
		*m_undoAction,
		*m_redoAction,
		*m_showImageViewport,
		*m_chooseWidget;
	QLabel* m_timeLabel;
};

#endif
