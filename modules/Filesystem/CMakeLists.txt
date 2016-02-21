cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Filesystem")
project(${PROJECT_NAME})

set(HEADER_FILES
)

set(SOURCE_FILES
	filesystemModule.cpp
	EnumerateFiles.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
