cmake_minimum_required(VERSION 3.1)

# Add all subdirectories containing a CMakeLists.txt
file(GLOB_RECURSE new_list RELATIVE ${CMAKE_CURRENT_LIST_DIR} CMakeLists.txt)
foreach(file_path ${new_list})
	if(NOT ${file_path} STREQUAL "CMakeLists.txt") # Not the one in modules/
		get_filename_component(dir_path ${file_path} DIRECTORY)
		add_subdirectory(${dir_path})
	endif()
endforeach()