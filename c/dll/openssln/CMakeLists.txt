cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("openssln")

set	(	
		SOURCES
		openssln.cpp
		)

set	(
		HEADERS
		)
add_library (openssln SHARED ${SOURCES} ${HEADERS} ../dll.def)

target_link_libraries(openssln openssll nspcl adtl ccl sysl)
target_link_libraries(openssln ${OPENSSL_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})

# OpenCV
# if (WIN32)
# 	target_link_libraries(openssln ws2_32 iphlpapi)
# endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER dll)

if (UNIX)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY 
			LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/../../lib/ccl/ccl.version")
endif()

# Installation
install	(
			TARGETS openssln 
			RUNTIME DESTINATION bin 
			LIBRARY DESTINATION bin
			ARCHIVE DESTINATION bin
			)

if (MSVC)
	install	(
				FILES				${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/openssln.pdb
				DESTINATION		bin
				CONFIGURATIONS	Debug
				)
endif()
