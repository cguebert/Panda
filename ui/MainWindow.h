#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <panda/messaging.h>
#include <memory>
#include <vector>

class QAction;
class QDockWidget;
class QLabel;
class QScrollArea;

class DatasTable;
class DataWatcher;
class DetachableTabWidget;
class DetachableWidgetInfo;
class DetachedWindow;
class ImageViewport;
class LayersTab;
class OpenGLRenderView;
class SimpleGUIImpl;
class ScrollContainer;
class UpdateLoggerDialog;

namespace panda {
	class BaseData;
	class PandaDocument;
	class PandaObject;
}

namespace graphview {
	class QtViewWrapper;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

	OpenGLRenderView* getOpenGLView() const;

	void fillContextMenu(QMenu& menu, int flags) const;
	void showStatusBarMessage(QString);

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
	void switchToDocumentView();
	void switchToOpenGLView();
	void toggleFullScreen(bool);
	void group();
	void ungroup();
	void editGroup();
	void saveGroup();
	void openGroup();
	void createGroupObject();
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
	void closeGraphView(graphview::QtViewWrapper* view);
	void convertSavedDocuments();
	void removedObject(panda::PandaObject*);
	void onTabWidgetFocusLoss(QWidget*);
	void onTabWidgetCloseTab(QWidget*);
	void onTabChanged();

private:
	void createActions();
	void createRegistryMenu();
	void createGroupRegistryMenu();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	bool okToContinue();
	bool loadFile(const QString& fileName);
	bool importFile(const QString& fileName);
	bool saveFile(const QString& fileName);
	void setCurrentFile(const QString& fileName);
	void updateRecentFileActions();
	QString strippedName(const QString& fullFileName);

	void undoEnabled(bool enabled);
	void redoEnabled(bool enabled);
	void undoTextChanged(const std::string& text);
	void redoTextChanged(const std::string& text);

	void closeTab(QWidget* container);

	QWidget* selectedTabWidget() const; // Return either the render view, a graph view or an image viewport

	void setDocument(const std::shared_ptr<panda::PandaDocument>& document);
	void updateAddObjectActions(QMenu* menu);
	void updateAddGroupActions(QMenu* menu);

	DetachableTabWidget* m_tabWidget = nullptr;
	graphview::QtViewWrapper* m_documentView = nullptr;
	ScrollContainer* m_documentViewContainer = nullptr;
	graphview::QtViewWrapper* m_currentGraphView = nullptr;
	OpenGLRenderView* m_openGLRenderView = nullptr;
	QScrollArea* m_openGLViewContainer = nullptr;
	std::shared_ptr<panda::PandaDocument> m_document;
	DatasTable* m_datasTable = nullptr;
	QDockWidget* m_datasDock = nullptr;
	LayersTab* m_layersTab = nullptr;
	QDockWidget* m_layersDock = nullptr;
	UpdateLoggerDialog* m_loggerDialog = nullptr;
	SimpleGUIImpl* m_simpleGUI = nullptr;

	QStringList m_recentFiles;
	QString m_curFile;

	bool m_fullScreen = false;
	bool m_exitFullscreenOnFocusLoss = true;
	bool m_adjustRenderSizeToView = false;
	bool m_undoEnabled = false;
	bool m_redoEnabled = false;

	panda::msg::Observer m_observer;

	QList<DetachedWindow*> m_detachedWindows;
	struct ImageViewportInfo
	{
		ImageViewport* viewport = nullptr;
		QWidget* container = nullptr;
		const panda::BaseData* data = nullptr;
		const panda::PandaObject* object = nullptr;
	};
	std::vector<ImageViewportInfo> m_imageViewports;

	struct GraphViewInfo
	{
		graphview::QtViewWrapper* view = nullptr;
		QWidget* container = nullptr;
		DetachableWidgetInfo* detachableInfo = nullptr;
		const panda::PandaObject* object = nullptr;
		std::shared_ptr<DataWatcher> nameWatcher;
	};
	std::vector<GraphViewInfo> m_graphViews;

	static const int MaxRecentFiles = 5;
	QAction* m_recentFileActions[MaxRecentFiles];
	QAction* m_separatorAction;
	std::vector<QAction*> m_allViewsActions; // The list of actions that apply to all views (GraphView & ImageViewport)
	std::vector<QAction*> m_graphViewsActions; // Actions that apply to GraphView

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
		*m_openGroupAction,
		*m_removeLinkAction,
		*m_copyDataAction,
		*m_showDirtyInfoAction,
		*m_undoAction,
		*m_redoAction,
		*m_showImageViewportAction,
		*m_setDataLabelAction,
		*m_chooseWidgetAction,
		*m_objectToBackAction,
		*m_objectToFrontAction;
	QLabel* m_timeLabel;
};

inline OpenGLRenderView* MainWindow::getOpenGLView() const
{ return m_openGLRenderView; }

#endif
