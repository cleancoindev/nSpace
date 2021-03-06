cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("imagen")

set	(	
		SOURCES
		imagen.cpp
		)

set	(
		HEADERS
		)
add_library (imagen SHARED ${SOURCES} ${HEADERS} ../dll.def)

target_link_libraries(imagen imagel mathl nspcl adtl ccl sysl)
if (MSVC)
target_link_libraries(imagen libjpeg libpng zlib)
endif()
if (UNIX)
target_link_libraries(imagen jpeg png)
target_link_libraries(imagen ${ZLIB_LIBRARIES})
endif()
target_link_libraries(imagen ${OpenCV_LIBS})

include_directories(${OpenCV_INCLUDE_DIRS})

# OpenCV
# if (WIN32)
# 	target_link_libraries(imagen ws2_32 iphlpapi)
# endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER dll)

if (UNIX)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY 
			LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/../../lib/ccl/ccl.version")
endif()

# OpenCV contrib build detection
# Get the first OpenCV include directory and look for a 'contrib' header
list (GET OpenCV_INCLUDE_DIRS 0 OpenCV_INCLUDE_DIR_FIRST)
if(EXISTS ${OpenCV_INCLUDE_DIR_FIRST}/opencv2/face/facerec.hpp)
	add_definitions(-DHAVE_OPENCV_CONTRIB)
endif()

# Installation
install	(
			TARGETS imagen 
			RUNTIME DESTINATION bin 
			LIBRARY DESTINATION bin
			ARCHIVE DESTINATION bin
			)

if (MSVC)
	install	(
				FILES				${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/imagen.pdb
				DESTINATION		bin
				CONFIGURATIONS	Debug
				)
endif()
