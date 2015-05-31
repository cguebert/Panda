TEMPLATE = lib
CONFIG += dll c++11
QT = widgets xml core
INCLUDEPATH = .
Release:TARGET = PandaCore
Debug:TARGET = PandaCore_d
DESTDIR = ../bin
DEFINES +=	PANDA_BUILD_CORE \
			PANDA_LOG_EVENTS \
			_CRT_SECURE_NO_WARNINGS \
			_SCL_SECURE_NO_WARNINGS

# Remember to modify boost path in modules/module.pri
BOOST="../../boost_1_58_0/"

INCLUDEPATH += $${BOOST}/
LIBPATH += $${BOOST}/stage/lib

PRECOMPILED_HEADER = panda/pch.h

HEADERS += \
	panda/BaseClass.h \
	panda/BaseData.h \
	panda/core.h \
	panda/Data.h \
	panda/DataAccessor.h \
	panda/DataCopier.h \
	panda/DataFactory.h \
	panda/DataNode.h \
	panda/Dockable.h \
	panda/GenericObject.h \
	panda/Group.h \
	panda/Layer.h \
	panda/ObjectFactory.h \
	panda/OGLObject.h \
	panda/PandaDocument.h \
	panda/PandaObject.h \
	panda/Renderer.h \
	panda/Scheduler.h \
	panda/command/CommandId.h \
	panda/command/DockableCommand.h \
	panda/command/GroupCommand.h \
	panda/command/LinkDatasCommand.h \
	panda/command/MoveLayerCommand.h \
	panda/command/RemoveGenericDataCommand.h \
	panda/helper/algorithm.h \
	panda/helper/Factory.h \
	panda/helper/gl.h \
	panda/helper/GradientCache.h \
	panda/helper/Perlin.h \
	panda/helper/PointsGrid.h \
	panda/helper/Random.h \
	panda/helper/typeList.h \
	panda/helper/ShaderCache.h \
	panda/helper/UpdateLogger.h \
	panda/helper/system/config.h \
	panda/helper/system/FileRepository.h \
	panda/types/Animation.h \
	panda/types/AnimationTraits.h \
	panda/types/Color.h \
	panda/types/DataTraits.h \
	panda/types/DataTypeId.h \
	panda/types/Gradient.h \
	panda/types/ImageWrapper.h \
	panda/types/Mesh.h \
	panda/types/Path.h \
	panda/types/Point.h \
	panda/types/Polygon.h \
	panda/types/Rect.h \
	panda/types/Shader.h \
	panda/types/StandardTypes.h \
	panda/types/TypeConverter.h

SOURCES += \
	panda/BaseClass.cpp \
	panda/BaseData.cpp \
	panda/core.cpp \
	panda/DataCopier.cpp \
	panda/DataFactory.cpp \
	panda/DataNode.cpp \
	panda/Dockable.cpp \
	panda/GenericObject.cpp \
	panda/Group.cpp \
	panda/Layer.cpp \
	panda/ObjectFactory.cpp \
	panda/OGLObject.cpp \
	panda/PandaDocument.cpp \
	panda/PandaObject.cpp \
	panda/Renderer.cpp \
	panda/Scheduler.cpp \
	panda/command/CommandId.cpp \
	panda/command/DockableCommand.cpp \
	panda/command/GroupCommand.cpp \
	panda/command/LinkDatasCommand.cpp \
	panda/command/MoveLayerCommand.cpp \
	panda/command/RemoveGenericDataCommand.cpp \
	panda/helper/Factory.cpp \
	panda/helper/GradientCache.cpp \
	panda/helper/Perlin.cpp \
	panda/helper/PointsGrid.cpp \
	panda/helper/Random.cpp \
	panda/helper/ShaderCache.cpp \
	panda/helper/UpdateLogger.cpp \
	panda/helper/system/FileRepository.cpp \
	panda/types/Animation.inl \
	panda/types/Color.cpp \
	panda/types/DataTraits.cpp \
	panda/types/DataTypeId.cpp \
	panda/types/Gradient.cpp \
	panda/types/ImageWrapper.cpp \
	panda/types/Mesh.cpp \
	panda/types/Path.cpp \
	panda/types/Point.cpp \
	panda/types/Polygon.cpp \
	panda/types/Rect.cpp \
	panda/types/Shader.cpp \
	panda/types/StandardTypes.cpp \
	panda/types/TypeConverter.cpp

unix {
	target.path = /usr/lib
	INSTALLS += target
}
