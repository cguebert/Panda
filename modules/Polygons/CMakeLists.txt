cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Polygons")
project(${PROJECT_NAME})

set(HEADER_FILES
	libs/clip2tri/clip2tri.h
	libs/clipper/clipper.hpp
	libs/poly2tri/poly2tri.h
	libs/poly2tri/common/shapes.h
	libs/poly2tri/common/utils.h
	libs/poly2tri/sweep/advancing_front.h
	libs/poly2tri/sweep/cdt.h
	libs/poly2tri/sweep/sweep.h
	libs/poly2tri/sweep/sweep_context.h
)

set(SOURCE_FILES
	ComposePolygon.cpp
	ConnectSegments.cpp
	MeshToPolygon.cpp
	PolygonMath.cpp
	PolygonOperations.cpp
	polygonsModule.cpp
	SimplePolygons.cpp
	Triangulation.cpp
	libs/clip2tri/clip2tri.cpp
	libs/clipper/clipper.cpp
	libs/poly2tri/common/shapes.cc
	libs/poly2tri/sweep/advancing_front.cc
	libs/poly2tri/sweep/cdt.cc
	libs/poly2tri/sweep/sweep.cc
	libs/poly2tri/sweep/sweep_context.cc
)

GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
