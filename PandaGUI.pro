TEMPLATE      = app
Release:TARGET = Panda
Debug:TARGET = Panda_d
HEADERS       = \
	ui/Annotation.h \
	ui/CreateGroup.h \
	ui/DatasTable.h \
	ui/EditGroupDialog.h \
	ui/GraphView.h \
	ui/GroupsManager.h \
	ui/LayersTab.h \
	ui/LinkTag.h \
	ui/ListObjectsAndTypes.h \
	ui/MainWindow.h \
	ui/OpenGLRenderView.h \
	ui/PluginsManager.h \
	ui/QuickCreateDialog.h \
	ui/UpdateLoggerDialog.h \
	ui/command/AddObjectCommand.h \
	ui/command/ModifyAnnotationCommand.h \
	ui/command/MoveObjectCommand.h \
	ui/command/RemoveObjectCommand.h \
	ui/command/SetDataValueCommand.h \
	ui/drawstruct/AnnotationDrawStruct.h \
	ui/drawstruct/DockableDrawStruct.h \
	ui/drawstruct/GenericObjectDrawStruct.h \
	ui/drawstruct/GroupDrawStruct.h \
	ui/drawstruct/LayerDrawStruct.h \
	ui/drawstruct/ObjectDrawStruct.h \
	ui/drawstruct/UserValueDrawStruct.h \
	ui/widget/AnimationDataWidgetDialog.h \
	ui/widget/DataWidget.h \
	ui/widget/DataWidgetFactory.h \
	ui/widget/EditGradientDialog.h \
	ui/widget/EditMeshDialog.h \
	ui/widget/EditShaderDialog.h \
	ui/widget/ListDataWidgetDialog.h \
	ui/widget/OpenDialogDataWidget.h \
	ui/widget/SimpleDataWidget.h \
	ui/widget/StructTraits.h \
	ui/widget/TableDataWidgetDialog.h
SOURCES       = ui/main.cpp \
	ui/Annotation.cpp \
	ui/CreateGroup.cpp \
	ui/DatasTable.cpp \
	ui/EditGroupDialog.cpp \
	ui/GraphView.cpp \
	ui/GroupsManager.cpp \
	ui/LayersTab.cpp \
	ui/LinkTag.cpp \
	ui/ListObjectsAndTypes.cpp \
	ui/MainWindow.cpp \
	ui/OpenGLRenderView.cpp \
	ui/PluginsManager.cpp \
	ui/QuickCreateDialog.cpp \
	ui/UpdateLoggerDialog.cpp \
	ui/command/AddObjectCommand.cpp \
	ui/command/ModifyAnnotationCommand.cpp \
	ui/command/MoveObjectCommand.cpp \
	ui/command/RemoveObjectCommand.cpp \
	ui/drawstruct/AnnotationDrawStruct.cpp \
	ui/drawstruct/DockableDrawStruct.cpp \
	ui/drawstruct/GenericObjectDrawStruct.cpp \
	ui/drawstruct/GroupDrawStruct.cpp \
	ui/drawstruct/LayerDrawStruct.cpp \
	ui/drawstruct/ObjectDrawStruct.cpp \
	ui/drawstruct/UserValueDrawStruct.cpp \
	ui/widget/AnimationDataWidgetDialog.cpp \
	ui/widget/ColorDataWidget.cpp \
	ui/widget/EditGradientDialog.cpp \
	ui/widget/EditMeshDialog.cpp \
	ui/widget/EditShaderDialog.cpp \
	ui/widget/DataWidgetFactory.cpp \
	ui/widget/NumericalDataWidget.cpp \
	ui/widget/PointDataWidget.cpp \
	ui/widget/RectDataWidget.cpp \
	ui/widget/StringDataWidget.cpp \
	ui/widget/TableDataWidgetDialog.cpp
RESOURCES     = panda.qrc
win32:RC_FILE = panda.rc
INCLUDEPATH = .
QT += widgets xml opengl
CONFIG += c++11
DEFINES +=	PANDA_LOG_EVENTS \
			_CRT_SECURE_NO_WARNINGS \
			_SCL_SECURE_NO_WARNINGS

DESTDIR = bin
Release:LIBS += bin/PandaCore.lib
Debug:LIBS += bin/PandaCore_d.lib

PRECOMPILED_HEADER = core/panda/pch.h

#DEFINES += PANDA_DOUBLE

BOOST="../boost_1_56_0"

INCLUDEPATH += $${BOOST}/
INCLUDEPATH += "core"
LIBPATH += $${BOOST}/stage/lib
