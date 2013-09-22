project(tinyxml)

cmake_minimum_required(VERSION 2.8)

set(SOURCES src/tinystr.cpp
            src/tinyxml.cpp
	    src/tinyxmlerror.cpp
	    src/tinyxmlparser.cpp)

add_definitions(-DWIN32 -D_LIB -DTIXML_USE_STL)

add_library(tinyxml ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/include)

set(HEADERS ${PROJECT_SOURCE_DIR}/include/tinystr.h
            ${PROJECT_SOURCE_DIR}/include/tinyxml.h)
install(FILES ${HEADERS} DESTINATION ${OUTPUT_DIR}/include)

install(TARGETS tinyxml DESTINATION ${OUTPUT_DIR}/lib)
