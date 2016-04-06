cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Meshes")
project(${PROJECT_NAME})

set(HEADER_FILES
)

set(SOURCE_FILES
	Centroids.cpp
	ComposeMesh.cpp
	Delaunay.cpp
	FindTriangle.cpp
	Grid.cpp
	MarchingSquares.cpp
	MergeMeshes.cpp
	meshesModule.cpp
	MeshInfo.cpp
	MeshMath.cpp
	Neighbors.cpp
	par_msquares.h
	Relaxation.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
