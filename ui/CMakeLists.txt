cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Panda")
project(${PROJECT_NAME})

set(QT_BASE_DIR "" CACHE PATH "Qt5 install prefix") # C:/Qt/5/msvc2015/
list(APPEND CMAKE_PREFIX_PATH ${QT_BASE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(HEADER_FILES
	CreateGroup.h
	DatasTable.h
	GroupsManager.h
	ImageViewport.h
	LayersTab.h
	ListObjectsAndTypes.h
	MainWindow.h
	OpenGLRenderView.h
	SimpleGUIImpl.h
	command/AddObjectCommand.h
	command/GroupSelectionCommand.h
	command/ModifyAnnotationCommand.h
	command/MoveObjectCommand.h
	command/RemoveObjectCommand.h
	command/SetDataValueCommand.h
	custom/DetachableTabWidget.h
	custom/ScrollContainer.h
	custom/qxt/QxtCheckComboBox.h
	dialog/ChooseWidgetDialog.h
	dialog/EditGroupDialog.h
	dialog/QuickCreateDialog.h
	dialog/UpdateLoggerDialog.h
	drawstruct/AnnotationDrawStruct.h
	drawstruct/DockableDrawStruct.h
	drawstruct/GenericObjectDrawStruct.h
	drawstruct/GroupDrawStruct.h
	drawstruct/LayerDrawStruct.h
	drawstruct/ObjectDrawStruct.h
	drawstruct/UserValueDrawStruct.h
	graphview/alignObjects.h
	graphview/GraphView.h
	graphview/LinkTag.h
	graphview/ObjectsSelection.h
	graphview/ViewRenderer.h
	graphview/graphics/DrawList.h
	widget/AnimationDataWidgetDialog.h
	widget/DataWidget.h
	widget/DataWidgetFactory.h
	widget/EditGradientDialog.h
	widget/EditMeshDialog.h
	widget/EditPolygonDialog.h
	widget/EditShaderDialog.h
	widget/ListDataWidgetDialog.h
	widget/NumericalDataWidget.h
	widget/OpenDialogDataWidget.h
	widget/SimpleDataWidget.h
	widget/StructTraits.h
	widget/TableDataWidgetDialog.h
)

set(SOURCE_FILES
	main.cpp
	CreateGroup.cpp
	DatasTable.cpp
	GroupsManager.cpp
	ImageViewport.cpp
	LayersTab.cpp
	ListObjectsAndTypes.cpp
	MainWindow.cpp
	OpenGLRenderView.cpp
	SimpleGUIImpl.cpp
	command/AddObjectCommand.cpp
	command/GroupSelectionCommand.cpp
	command/ModifyAnnotationCommand.cpp
	command/MoveObjectCommand.cpp
	command/RemoveObjectCommand.cpp
	custom/DetachableTabWidget.cpp
	custom/ScrollContainer.cpp
	custom/qxt/QxtCheckComboBox.cpp
	dialog/ChooseWidgetDialog.cpp
	dialog/EditGroupDialog.cpp
	dialog/QuickCreateDialog.cpp
	dialog/UpdateLoggerDialog.cpp
	drawstruct/AnnotationDrawStruct.cpp
	drawstruct/DockableDrawStruct.cpp
	drawstruct/GenericObjectDrawStruct.cpp
	drawstruct/GroupDrawStruct.cpp
	drawstruct/LayerDrawStruct.cpp
	drawstruct/ObjectDrawStruct.cpp
	drawstruct/UserValueDrawStruct.cpp
	graphview/alignObjects.cpp
	graphview/GraphView.cpp
	graphview/LinkTag.cpp
	graphview/ObjectsSelection.cpp
	graphview/ViewRenderer.cpp
	graphview/graphics/DrawList.cpp
	widget/ColorDataWidget.cpp
	widget/DataWidgetFactory.cpp
	widget/EditGradientDialog.cpp
	widget/EditMeshDialog.cpp
	widget/EditPolygonDialog.cpp
	widget/EditShaderDialog.cpp
	widget/FloatDataWidget.cpp
	widget/IntegerDataWidget.cpp
	widget/PointDataWidget.cpp
	widget/RectDataWidget.cpp
	widget/SourceCodeDataWidget.cpp
	widget/StringDataWidget.cpp
)

GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

EnablePrecompiledHeaders(SOURCE_FILES)

set(RESSOURCE_FILES ${PROJECT_NAME}.qrc)
if(WIN32)
	set(RESSOURCE_FILES ${RESSOURCE_FILES} ${PROJECT_NAME}.rc)
endif()

find_package(Qt5Widgets)
find_package(Qt5OpenGL)

if(WIN32)
	set(GUI_TYPE WIN32)
endif()

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${RESSOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Applications")

target_link_libraries(${PROJECT_NAME} "PandaCore")
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL)

add_library("QScintilla" SHARED IMPORTED)
set_target_properties("QScintilla" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/qscintilla2.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/qscintilla2_d.lib)
target_link_libraries(${PROJECT_NAME} "QScintilla")
install(FILES "${LIBRARIES_LIB_DIR}/qscintilla2.dll" CONFIGURATIONS Release RelWithDebInfo DESTINATION "${TARGET_DIR}")
install(FILES "${LIBRARIES_LIB_DIR}/qscintilla2_d.dll" CONFIGURATIONS Debug DESTINATION "${TARGET_DIR}")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TARGET_DIR})
install(DIRECTORY "../share/shaders" DESTINATION ${TARGET_DIR})
install(DIRECTORY "../share/groups" DESTINATION ${TARGET_DIR})

# Install Qt files
if(WIN32)
	set(QT_LIBS Qt5Core Qt5Gui Qt5OpenGL Qt5Widgets)
	
	set(qtLibsRelease)
	set(qtLibsDebug)
	
	foreach(libName ${QT_LIBS})
		list(APPEND qtLibsRelease	"${QT_BASE_DIR}/bin/${libName}.dll")
		list(APPEND qtLibsDebug		"${QT_BASE_DIR}/bin/${libName}d.dll")
	endforeach(libName)
	
	install(FILES ${qtLibsRelease} CONFIGURATIONS Release RelWithDebInfo DESTINATION ${TARGET_DIR})
	install(FILES ${qtLibsDebug} CONFIGURATIONS Debug DESTINATION ${TARGET_DIR})
	
	install(FILES "${QT_BASE_DIR}/plugins/platforms/qwindows.dll" CONFIGURATIONS Release RelWithDebInfo DESTINATION "${TARGET_DIR}/platforms")
	install(FILES "${QT_BASE_DIR}/plugins/platforms/qwindowsd.dll" CONFIGURATIONS Debug DESTINATION "${TARGET_DIR}/platforms")
endif()
