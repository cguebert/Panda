cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "PandaCore")
project(${PROJECT_NAME})

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

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
