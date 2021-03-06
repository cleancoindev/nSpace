cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("libjpeg")

set	(
		SOURCES
		cdjpeg.c
		jaricom.c
		jcapimin.c
		jcapistd.c
		jcarith.c
		jccoefct.c
		jccolor.c
		jcdctmgr.c
		jchuff.c
		jcinit.c
		jcmainct.c
		jcmarker.c
		jcmaster.c
		jcomapi.c
		jcparam.c
		jcprepct.c
		jcsample.c
		jctrans.c
		jdapimin.c
		jdapistd.c
		jdarith.c
		jdatadst.c
		jdatasrc.c
		jdcoefct.c
		jdcolor.c
		jddctmgr.c
		jdhuff.c
		jdinput.c
		jdmainct.c
		jdmarker.c
		jdmaster.c
		jdmerge.c
		jdpostct.c
		jdsample.c
		jdtrans.c
		jerror.c
		jfdctflt.c
		jfdctfst.c
		jfdctint.c
		jidctflt.c
		jidctfst.c
		jidctint.c
		jmemansi.c
		jmemmgr.c
		jquant1.c
		jquant2.c
		jutils.c
		rdbmp.c
		rdcolmap.c
		rdgif.c
		rdppm.c
		rdrle.c
		rdswitch.c
		rdtarga.c
		transupp.c
		wrbmp.c
		wrgif.c
		wrjpgcom.c
		wrppm.c
		wrrle.c
		wrtarga.c
		)

set	(
		HEADERS
		cderror.h
		cdjpeg.h
		jconfig.h
		jdct.h
		jerror.h
		jinclude.h
		jmemsys.h
		jmorecfg.h
		jpegint.h
		jpeglib.h
		jversion.h
		transupp.h
		)

add_library (libjpeg STATIC ${SOURCES} ${HEADERS})

target_link_libraries(libjpeg)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lib/ext)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# Installation
install	(
			TARGETS libjpeg
			RUNTIME DESTINATION bin 
			LIBRARY DESTINATION lib 
			ARCHIVE DESTINATION lib 
			)
