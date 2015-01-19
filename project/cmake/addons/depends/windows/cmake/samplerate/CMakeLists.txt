project(samplerate)

cmake_minimum_required(VERSION 2.8)

set(SOURCES src/samplerate.c
            src/src_linear.c
            src/src_sinc.c
            src/src_zoh.c)

add_definitions(-DWIN32 -D_LIB)

add_library(samplerate ${SOURCES})

include_directories(	${PROJECT_SOURCE_DIR}/src
						${PROJECT_SOURCE_DIR}/Win32)
						
if(WIN32)
	include_directories(	${PROJECT_SOURCE_DIR}/src
							${PROJECT_SOURCE_DIR}/Win32)
else()
	include_directories(	${PROJECT_SOURCE_DIR}/src)
endif()

set(HEADERS ${PROJECT_SOURCE_DIR}/src/samplerate.h
            ${PROJECT_SOURCE_DIR}/src/float_cast.h
			${PROJECT_SOURCE_DIR}/src/common.h)
			
if(WIN32)
	list(APPEND HEADERS "${PROJECT_SOURCE_DIR}/Win32/config.h")
endif()
			
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/samplerate)

install(TARGETS samplerate DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
