cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("mathn")

set	(	
		SOURCES
		mathn.cpp
		)

set	(
		HEADERS
		)

add_library (mathn SHARED ${SOURCES} ${HEADERS} ../dll.def)

target_link_libraries(mathn mathl nspcl adtl ccl sysl)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER dll)

if (UNIX)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY 
			LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/../../lib/ccl/ccl.version")
endif()

# Installation
install	(
			TARGETS mathn 
			RUNTIME DESTINATION bin 
			LIBRARY DESTINATION bin
			ARCHIVE DESTINATION bin
			)


if (MSVC)
	install	(
				FILES				${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/mathn.pdb
				DESTINATION		bin
				CONFIGURATIONS	Debug
				)
endif()
