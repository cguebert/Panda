cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME "PandaViewer")

project(${PROJECT_NAME})

set(HEADER_FILES
	SimpleGUIImpl.h
)

set(SOURCE_FILES
	main.cpp
	SimpleGUIImpl.cpp
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Applications")

target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIR}/glfw3.lib")
target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIR}/glew32.lib")

target_link_libraries(${PROJECT_NAME} "PandaCore")

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TARGET_DIR})