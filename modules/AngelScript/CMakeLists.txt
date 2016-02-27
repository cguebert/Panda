cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "AngelScript")
project(${PROJECT_NAME})

set(HEADER_FILES
	addons/scriptarray/scriptarray.h
	addons/scriptstdstring/scriptstdstring.h
)

set(SOURCE_FILES
	angelScriptModule.cpp
	AS_Object.cpp
	addons/scriptarray/scriptarray.cpp
	addons/scriptstdstring/scriptstdstring.cpp
	addons/scriptstdstring/scriptstdstring_utils.cpp
)

GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

add_library("AngelScriptLib" STATIC IMPORTED)
set_target_properties("AngelScriptLib" PROPERTIES 
	IMPORTED_LOCATION ${LIBRARIES_LIB_DIR}/angelscript.lib
	IMPORTED_LOCATION_DEBUG ${LIBRARIES_LIB_DIR}/angelscriptd.lib)
target_link_libraries(${PROJECT_NAME} "AngelScriptLib")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")