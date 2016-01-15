cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "PandaSolution")
project(${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/functions.cmake)
include_directories(".")
include_directories("extlibs")

set(LIBRARIES_BASE_DIR "" CACHE PATH "Directory where other libraries are installed")
set(LIBRARIES_INCLUDE_DIR ${LIBRARIES_BASE_DIR}/include)
set(LIBRARIES_LIB_DIR ${LIBRARIES_BASE_DIR}/lib)
include_directories(${LIBRARIES_INCLUDE_DIR})

set(BOOST_BASE_DIR "" CACHE PATH "Root directory of the Boost librairies")
set(USE_STATIC_BOOST ON CACHE BOOL "Use the static version of the Boost librairies")
set(Boost_USE_STATIC_LIBS ${USE_STATIC_BOOST})
set(Boost_USE_MULTITHREADED ON)
add_definitions(-DBOOST_ALL_NO_LIB)
set(BOOST_ROOT ${BOOST_BASE_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(QT_BASE_DIR "" CACHE PATH "Qt5 install prefix") # C:/Qt/5/msvc2015/
list(APPEND CMAKE_PREFIX_PATH ${QT_BASE_DIR})

set(TARGET_DIR "${CMAKE_INSTALL_PREFIX}/\${BUILD_TYPE}")
set(MODULES_DIR "${TARGET_DIR}/modules")

# Multiprocessor build on Visual C++
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_definitions(-DPANDA_LOG_EVENTS)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

# Core
include_directories("core")
add_subdirectory("core")

# Modules
add_subdirectory("modules")

# Application
add_subdirectory("ui")
