cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Paths")
project(${PROJECT_NAME})

set(HEADER_FILES
)

set(SOURCE_FILES
	BezierSpline.cpp
	CubicBSpline.cpp
	Extrude.cpp
	PathMath.cpp
	pathsModule.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
