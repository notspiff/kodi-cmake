set(SOURCES src/md5.cpp
            src/XBTFWriter.cpp
            src/TexturePacker.cpp
            src/DecoderManager.cpp
            src/decoder/JPGDecoder.cpp
            src/decoder/PNGDecoder.cpp
            src/decoder/GifHelper.cpp
            src/decoder/GIFDecoder.cpp
            ${CORE_SOURCE_DIR}/xbmc/guilib/XBTF.cpp)

include(addoptions)
include(macros)
add_options(CXX ALL_BUILDS "-fPIC")

# Fixme
add_definitions(-DTARGET_POSIX -DUSE_LZO_PACKING)

list(APPEND CMAKE_MODULE_PATH ${CORE_SOURCE_DIR}/project/cmake/modules)

find_package(Lzo2 REQUIRED)
find_package(PNG REQUIRED)
find_package(GIF REQUIRED)
find_package(JPEG REQUIRED)

if(GIF_VERSION LESS 4)
  message(FATAL_ERROR "giflib < 4 not supported")
else()
  file(STRINGS ${GIF_INCLUDE_DIR}/gif_lib.h GIFSTRINGS)
  string(REGEX MATCH GIFLIB_MAJOR ([0-9]*) GIFLIB_MAJOR ${GIFSTRINGS})
  if(NOT GIFLIB_MAJOR)
    message(WARNING "giflib4 support is experimental. Consider updating to giflib5")
  endif()
endif()

set(SQUISH_NATIVE 1)
add_subdirectory(${CORE_SOURCE_DIR}/tools/depends/native/libsquish-native squish-native)
set(SQUISH_NATIVE)

include_directories(${PNG_INCLUDE_DIRS}
                    ${JPEG_INCLUDE_DIR}
                    ${GIF_INCLUDE_DIR}
                    ${CORE_SOURCE_DIR}/xbmc
                    ${CORE_SOURCE_DIR}/lib
                    ${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/decoder)
add_executable(TexturePacker ${SOURCES})
target_link_libraries(TexturePacker ${SYSTEM_LDFLAGS} squish-native
                                    ${GIF_LIBRARIES}
                                    ${PNG_LIBRARIES}
                                    ${JPEG_LIBRARIES}
                                    ${LZO2_LIBRARIES})
