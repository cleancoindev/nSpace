# Solution folder
# set_target_properties(${PROJECT_NAME} PROPERTIES USE_FOLDERS Test)

add_subdirectory(adtn)
add_subdirectory(ion)
add_subdirectory(mathn)
add_subdirectory(median)
add_subdirectory(miscn)
add_subdirectory(netn)
add_subdirectory(nspcn)
add_subdirectory(sysn)

if(MSVC)
add_subdirectory(plotn)
add_subdirectory(sqln)
add_subdirectory(usbn)
add_subdirectory(wmin)
endif()

# External SDKs
if( OpenCV_FOUND )
	add_subdirectory(imagen)
endif()

if( OpenSSL_FOUND )
	add_subdirectory(openssln)
endif()

if ( VTK_FOUND )
	add_subdirectory(visualizen)
endif()
