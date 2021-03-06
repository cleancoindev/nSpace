cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("sqln")

set	(	
		SOURCES
		sqln.cpp
		)

set	(
		HEADERS
		)

add_library (sqln SHARED ${SOURCES} ${HEADERS} ../dll.def)

target_link_libraries(sqln sqll adtl nspcl sysl ccl)
target_link_libraries(sqln odbc32)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER dll)

if (UNIX)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY 
			LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/../../lib/ccl/ccl.version")
endif()

# Installation
install	(
			TARGETS sqln 
			RUNTIME DESTINATION bin 
			LIBRARY DESTINATION bin
			ARCHIVE DESTINATION bin
			)

if (MSVC)
	install	(
				FILES				${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/sqln.pdb
				DESTINATION		bin
				CONFIGURATIONS	Debug
				)
endif()
