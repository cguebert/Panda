cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "PandaCore")
project(${PROJECT_NAME})

set(QT_BASE_DIR "" CACHE PATH "Qt5 install prefix") # C:/Qt/5/msvc2015/
list(APPEND CMAKE_PREFIX_PATH ${QT_BASE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(HEADER_FILES
	panda/BaseClass.h
	panda/BaseData.h
	panda/core.h
	panda/Data.h
	panda/DataAccessor.h
	panda/DataCopier.h
	panda/DataFactory.h
	panda/DataNode.h
	panda/Dockable.h
	panda/GenericObject.h
	panda/Group.h
	panda/Layer.h
	panda/messaging.h
	panda/ObjectFactory.h
	panda/OGLObject.h
	panda/PandaDocument.h
	panda/PandaObject.h
	panda/PluginsManager.h
	panda/Renderer.h
	panda/Scheduler.h
	panda/XmlDocument.h
	panda/command/CommandId.h
	panda/command/DockableCommand.h
	panda/command/GroupCommand.h
	panda/command/LinkDatasCommand.h
	panda/command/MoveLayerCommand.h
	panda/command/RemoveGenericDataCommand.h
	panda/graphics/Framebuffer.h
	panda/graphics/Image.h
	panda/graphics/ShaderProgram.h
	panda/graphics/Texture.h
	panda/helper/algorithm.h
	panda/helper/gl.h
	panda/helper/GradientCache.h
	panda/helper/Perlin.h
	panda/helper/PointsGrid.h
	panda/helper/Random.h
	panda/helper/typeList.h
	panda/helper/ShaderCache.h
	panda/helper/UpdateLogger.h
	panda/helper/system/config.h
	panda/helper/system/FileRepository.h
	panda/types/Animation.h
	panda/types/AnimationTraits.h
	panda/types/Color.h
	panda/types/DataTraits.h
	panda/types/DataTypeId.h
	panda/types/Gradient.h
	panda/types/ImageWrapper.h
	panda/types/Mesh.h
	panda/types/Path.h
	panda/types/Point.h
	panda/types/Polygon.h
	panda/types/Rect.h
	panda/types/Shader.h
	panda/types/StandardTypes.h
	panda/types/TypeConverter.h
)

set(SOURCE_FILES
	panda/BaseClass.cpp
	panda/BaseData.cpp
	panda/core.cpp
	panda/DataCopier.cpp
	panda/DataFactory.cpp
	panda/DataNode.cpp
	panda/Dockable.cpp
	panda/GenericObject.cpp
	panda/Group.cpp
	panda/Layer.cpp
	panda/ObjectFactory.cpp
	panda/OGLObject.cpp
	panda/PandaDocument.cpp
	panda/PandaObject.cpp
	panda/PluginsManager.cpp
	panda/Renderer.cpp
	panda/Scheduler.cpp
	panda/XmlDocument.cpp
	panda/command/CommandId.cpp
	panda/command/DockableCommand.cpp
	panda/command/GroupCommand.cpp
	panda/command/LinkDatasCommand.cpp
	panda/command/MoveLayerCommand.cpp
	panda/command/RemoveGenericDataCommand.cpp
	panda/graphics/Framebuffer.cpp
	panda/graphics/Image.cpp
	panda/graphics/ShaderProgram.cpp
	panda/graphics/Texture.cpp
	panda/helper/GradientCache.cpp
	panda/helper/Perlin.cpp
	panda/helper/PointsGrid.cpp
	panda/helper/Random.cpp
	panda/helper/ShaderCache.cpp
	panda/helper/UpdateLogger.cpp
	panda/helper/system/FileRepository.cpp
	panda/types/Animation.inl
	panda/types/Color.cpp
	panda/types/DataTraits.cpp
	panda/types/DataTypeId.cpp
	panda/types/Gradient.cpp
	panda/types/ImageWrapper.cpp
	panda/types/Mesh.cpp
	panda/types/Path.cpp
	panda/types/Point.cpp
	panda/types/Polygon.cpp
	panda/types/Rect.cpp
	panda/types/Shader.cpp
	panda/types/StandardTypes.cpp
	panda/types/TypeConverter.cpp
)

# Regroup files by folder
GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(Qt5Xml)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

#set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")

add_definitions(-DPANDA_BUILD_CORE)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL)

add_library("TinyXML2" SHARED IMPORTED)
set_target_properties("TinyXML2" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/tinyxml2.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/tinyxml2d.lib)
target_link_libraries(${PROJECT_NAME} "TinyXML2")

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
 
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIR}/glew32.lib")

install(FILES ${LIBRARIES_LIB_DIR}/glew32.dll DESTINATION ${TARGET_DIR})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TARGET_DIR})
