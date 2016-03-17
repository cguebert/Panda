cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Text")
project(${PROJECT_NAME})

set(HEADER_FILES
	FontsCache.h
	TextureFont.h
)

set(SOURCE_FILES
	textModule.cpp
	FontsCache.cpp
	RenderText.cpp
	TextPosition.cpp
	TextureFont.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

add_library("freetype" SHARED IMPORTED)
set_target_properties("freetype" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/freetype.lib)
target_link_libraries(${PROJECT_NAME} "freetype")

add_library("freetype-gl" SHARED IMPORTED)
set_target_properties("freetype-gl" PROPERTIES 
	IMPORTED_IMPLIB ${LIBRARIES_LIB_DIR}/freetype-gl.lib
	IMPORTED_IMPLIB_DEBUG ${LIBRARIES_LIB_DIR}/freetype-gl_d.lib)
target_link_libraries(${PROJECT_NAME} "freetype-gl")

target_link_libraries(${PROJECT_NAME} "legacy_stdio_definitions.lib")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
