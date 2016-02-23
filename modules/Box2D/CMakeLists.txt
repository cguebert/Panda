cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Box2D")
project(${PROJECT_NAME})

set(HEADER_FILES
	World.h
)

set(SOURCE_FILES
	Bodies.cpp
	World.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

target_link_libraries(${PROJECT_NAME} optimized "${LIBRARIES_LIB_DIR}/liquidfun.lib" debug "${LIBRARIES_LIB_DIR}/liquidfun_d.lib")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
