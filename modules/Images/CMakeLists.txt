cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "Images")
project(${PROJECT_NAME})

set(HEADER_FILES
	utils.h
)

set(SOURCE_FILES
	imagesModule.cpp
	ImageExtensions.cpp
	LoadImage.cpp
	ResizeImage.cpp
	SaveImage.cpp
	utils.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

target_link_libraries(${PROJECT_NAME} "${LIBRARIES_LIB_DIR}/FreeImage.lib")
install(FILES ${LIBRARIES_LIB_DIR}/FreeImage.dll DESTINATION ${TARGET_DIR})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
