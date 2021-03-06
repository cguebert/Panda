cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "List")
project(${PROJECT_NAME})

set(HEADER_FILES
)

set(SOURCE_FILES
	Concatenation.cpp
	Condition.cpp
	DataCounter.cpp
	DeleteItems.cpp
	ExtractHead.cpp
	ExtractTail.cpp
	FillList.cpp
	FilterItems.cpp
	FindItem.cpp
	IndicesOperations.cpp
	InterleaveSequence.cpp
	ListBuffer.cpp
	ListEqual.cpp
	ListItem.cpp
	ListLoop.cpp
	ListMinMax.cpp
	listModule.cpp
	ListReverse.cpp
	ListSize.cpp
	ReplaceItem.cpp
	RemoveDuplicates.cpp
	SortList.cpp
	Switch.cpp
	VectorsListTranspose.cpp
)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
