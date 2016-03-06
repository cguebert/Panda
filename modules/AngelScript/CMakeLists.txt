cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "AngelScript")
project(${PROJECT_NAME})

set(HEADER_FILES
	ScriptEngine.h
	addons/scriptarray/scriptarray.h
	addons/scriptmath/scriptmath.h
	addons/scriptstdstring/scriptstdstring.h
	addons/aatc/aatc.hpp
	addons/aatc/aatc_common.hpp
	addons/aatc/aatc_config.hpp
	addons/aatc/aatc_container_list.hpp
	addons/aatc/aatc_container_listing.hpp
	addons/aatc/aatc_container_listing_heavy.hpp
	addons/aatc/aatc_container_map.hpp
	addons/aatc/aatc_container_mapped_templated_shared.hpp
	addons/aatc/aatc_container_mapped_templated_shared_method.hpp
	addons/aatc/aatc_container_set.hpp
	addons/aatc/aatc_container_shared.hpp
	addons/aatc/aatc_container_templated_shared.hpp
	addons/aatc/aatc_container_templated_shared_method.hpp
	addons/aatc/aatc_container_tempspec_shared.hpp
	addons/aatc/aatc_container_tempspec_shared_method.hpp
	addons/aatc/aatc_container_unordered_map.hpp
	addons/aatc/aatc_container_unordered_set.hpp
	addons/aatc/aatc_container_vector.hpp
	addons/aatc/aatc_enginestorage.hpp
	addons/aatc/aatc_hash.hpp
	addons/aatc/aatc_serializer.hpp
	addons/aatc/aatc_templatemagic.hpp
	pandaTypes/DataWrapper.h
	pandaTypes/ObjectWrapper.h
	pandaTypes/Types.h
)

set(SOURCE_FILES
	angelScriptModule.cpp
	AS_Object.cpp
	ScriptEngine.cpp
	addons/scriptarray/scriptarray.cpp
	addons/scriptmath/scriptmath.cpp
	addons/scriptstdstring/scriptstdstring.cpp
	addons/scriptstdstring/scriptstdstring_utils.cpp
	addons/aatc/aatc.cpp
	addons/aatc/aatc_common.cpp
	addons/aatc/aatc_container_list.cpp
	addons/aatc/aatc_container_map.cpp
	addons/aatc/aatc_container_set.cpp
	addons/aatc/aatc_container_shared.cpp
	addons/aatc/aatc_container_unordered_map.cpp
	addons/aatc/aatc_container_unordered_set.cpp
	addons/aatc/aatc_container_vector.cpp
	addons/aatc/aatc_enginestorage.cpp
	addons/aatc/aatc_hash.cpp
	addons/aatc/aatc_registration.cpp
	addons/aatc/aatc_serializer.cpp
	pandaTypes/Color.cpp
	pandaTypes/Gradient.cpp
	pandaTypes/Point.cpp
	pandaTypes/Rect.cpp
	pandaTypes/Types.cpp
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
