cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "UserValue")
project(${PROJECT_NAME})

set(HEADER_FILES
	LoadValue.h
	StoreValue.h
	UserValue.h
)

set(SOURCE_FILES
	LoadValue.cpp
	StoreValue.cpp
	UserValue.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

find_package(Qt5Widgets)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
