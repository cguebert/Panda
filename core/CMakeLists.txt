cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "PandaCore")
project(${PROJECT_NAME})

set(HEADER_FILES
	panda/core.h
	panda/messaging.h
	panda/PandaDocument.h
	panda/PluginsManager.h
	panda/Scheduler.h
	panda/SimpleGUI.h
	panda/TimedFunctions.h
	panda/UpdateLogger.h
	panda/UndoStack.h
	panda/XmlDocument.h
	panda/command/CommandId.h
	panda/command/DockableCommand.h
	panda/command/GroupCommand.h
	panda/command/LinkDatasCommand.h
	panda/command/MoveLayerCommand.h
	panda/command/RemoveGenericDataCommand.h
	panda/data/BaseClass.h
	panda/data/BaseData.h
	panda/data/Data.h
	panda/data/DataAccessor.h
	panda/data/DataCopier.h
	panda/data/DataFactory.h
	panda/data/DataNode.h
	panda/document/DocumentRenderer.h
	panda/document/DocumentSignals.h
	panda/document/GraphUtils.h
	panda/document/Serialization.h
	panda/graphics/Buffer.h
	panda/graphics/Framebuffer.h
	panda/graphics/Image.h
	panda/graphics/Mat4x4.h
	panda/graphics/Model.h
	panda/graphics/PointInt.h
	panda/graphics/RectInt.h
	panda/graphics/ShaderProgram.h
	panda/graphics/Size.h
	panda/graphics/Texture.h
	panda/graphics/VertexArrayObject.h
	panda/helper/algorithm.h
	panda/helper/EasingFunctions.h
	panda/helper/Font.h
	panda/helper/GradientCache.h
	panda/helper/Perlin.h
	panda/helper/PointsGrid.h
	panda/helper/Random.h
	panda/helper/typeList.h
	panda/helper/ShaderCache.h
	panda/helper/system/config.h
	panda/helper/system/FileRepository.h
	panda/helper/system/pch.h
	panda/object/Annotation.h
	panda/object/Dockable.h
	panda/object/GenericObject.h
	panda/object/Group.h
	panda/object/Layer.h
	panda/object/ObjectFactory.h
	panda/object/OGLObject.h
	panda/object/PandaObject.h
	panda/object/Renderer.h
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
	panda/core.cpp
	panda/PandaDocument.cpp
	panda/PluginsManager.cpp
	panda/Scheduler.cpp
	panda/SimpleGUI.cpp
	panda/TimedFunctions.cpp
	panda/UpdateLogger.cpp
	panda/UndoStack.cpp
	panda/XmlDocument.cpp
	panda/command/CommandId.cpp
	panda/command/DockableCommand.cpp
	panda/command/GroupCommand.cpp
	panda/command/LinkDatasCommand.cpp
	panda/command/MoveLayerCommand.cpp
	panda/command/RemoveGenericDataCommand.cpp
	panda/data/BaseClass.cpp
	panda/data/BaseData.cpp
	panda/data/DataCopier.cpp
	panda/data/DataFactory.cpp
	panda/data/DataNode.cpp
	panda/document/DocumentRenderer.cpp
	panda/document/GraphUtils.cpp
	panda/document/Serialization.cpp
	panda/graphics/Buffer.cpp
	panda/graphics/Framebuffer.cpp
	panda/graphics/Image.cpp
	panda/graphics/Mat4x4.cpp
	panda/graphics/Model.cpp
	panda/graphics/ShaderProgram.cpp
	panda/graphics/Size.cpp
	panda/graphics/Texture.cpp
	panda/graphics/VertexArrayObject.cpp
	panda/helper/EasingFunctions.cpp
	panda/helper/Font.cpp
	panda/helper/GradientCache.cpp
	panda/helper/Perlin.cpp
	panda/helper/PointsGrid.cpp
	panda/helper/Random.cpp
	panda/helper/ShaderCache.cpp
	panda/helper/system/FileRepository.cpp
	panda/object/Annotation.cpp
	panda/object/Dockable.cpp
	panda/object/GenericObject.cpp
	panda/object/Group.cpp
	panda/object/Layer.cpp
	panda/object/ObjectFactory.cpp
	panda/object/OGLObject.cpp
	panda/object/PandaObject.cpp
	panda/object/Renderer.cpp
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

# Use a precompiled header
EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

#set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Libraries")

add_definitions(-DPANDA_BUILD_CORE)

add_library("TinyXML2" STATIC IMPORTED)
set_target_properties("TinyXML2" PROPERTIES 
	IMPORTED_LOCATION ${LIBRARIES_LIB_DIR}/tinyxml2.lib
	IMPORTED_LOCATION_DEBUG ${LIBRARIES_LIB_DIR}/tinyxml2d.lib)
target_link_libraries(${PROJECT_NAME} "TinyXML2")

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
 
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIR}/glew32.lib")
install(FILES ${LIBRARIES_LIB_DIR}/glew32.dll DESTINATION ${TARGET_DIR})

find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TARGET_DIR})
