cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Render")
project(${PROJECT_NAME})

set(HEADER_FILES
)

set(SOURCE_FILES
	renderModule.cpp
	RenderCircle.cpp
	RenderConvexPolygon.cpp
	RenderDisk.cpp
	RenderImage.cpp
	RenderFill.cpp
	RenderLine.cpp
	RenderMesh.cpp
	RenderRect.cpp
	RenderSprite.cpp
	RenderTriangle.cpp
	modifiers/RenderModifierBlend.cpp
	modifiers/RenderModifierTransformations.cpp
)

GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
